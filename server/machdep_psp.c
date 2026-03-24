#include <fhttpd.h>

#if defined(FPR_IS_PSP)
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <psputility.h>
#include <pspctrl.h>
#include <pspnet_apctl.h>
#include <pspwlan.h>

#define printf pspDebugScreenPrintf

PSP_MODULE_INFO("Feather HTTPd", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

static int psp_exit_callback(int arg1, int arg2, void* arg3) {
	running = fpr_false;
}

static int psp_callback_thread(SceSize args, void* argp) {
	int cid;
	cid = sceKernelCreateCallback("Exit Call Back", psp_exit_callback, NULL);
	sceKernelRegisterExitCallback(cid);
	sceKernelSleepThreadCB();
	return 0;
}

static void psp_net_init(void) {
	int		      i;
	int		      choice[100];
	int		      incr  = 0;
	int		      last  = 0;
	int		      cur   = 0;
	int		      press = 0;
	union SceNetApctlInfo info;
	for(i = 1; i < 100; i++) {
		choice[i - 1] = 0;
		netData name;
		netData data;
		if(sceUtilityCheckNetParam(i) != 0) continue;
		choice[incr++] = i;
		pspDebugScreenSetXY(0, 1 + 3 + incr - 1);
		if(incr == 1) printf("> ");
		pspDebugScreenSetXY(2, 1 + 3 + incr - 1);
		sceUtilityGetNetParam(i, 0, &name);
		sceUtilityGetNetParam(i, 1, &data);
		printf("SSID=%s", data.asString);
		sceUtilityGetNetParam(i, 4, &data);
		if(data.asString[0]) {
			sceUtilityGetNetParam(i, 5, &data);
			printf(" IPADDR=%s\n", data.asString);
		} else {
			printf(" DHCP\n");
		}
	}

	while(1) {
		if(!running) {
			sceKernelExitGame();
		}
		SceCtrlData c;
		sceCtrlReadBufferPositive(&c, 1);
		press = 0;
		if(c.Buttons & PSP_CTRL_DOWN) {
			if(cur < incr - 1) {
				cur++;
			}
			press = 1;
		} else if(c.Buttons & PSP_CTRL_UP) {
			if(cur > 0) {
				cur--;
			}
			press = -1;
		} else if(c.Buttons & PSP_CTRL_START) {
			break;
		}
		if(last != cur) {
			pspDebugScreenSetXY(0, 1 + 3 + last);
			printf("  ");
			pspDebugScreenSetXY(0, 1 + 3 + cur);
			printf("> ");
			last = cur;
		}
		if(press != 0) {
			while(1) {
				SceCtrlData c;
				sceCtrlReadBufferPositive(&c, 1);
				if(press == 1) {
					if(!(c.Buttons & PSP_CTRL_DOWN)) break;
				} else if(press == -1) {
					if(!(c.Buttons & PSP_CTRL_UP)) break;
				}
			}
		}
	}
	pspDebugScreenSetXY(0, 1 + 3 + incr + 1);
	int err = sceNetApctlConnect(choice[cur]);
	if(err != 0) {
		printf("Apctl initialization failure\n");
		while(running) sceKernelDelayThread(50 * 1000);
		sceKernelExitGame();
	} else {
		printf("Apctl initialization successful\n");
	}
	printf("Apctl connecting\n");
	while(1) {
		int state;
		err = sceNetApctlGetState(&state);
		if(err != 0) {
			printf("Apctl getting status failure\n");
			while(running) sceKernelDelayThread(50 * 1000);
			sceKernelExitGame();
		}
		if(state == 4) {
			break;
		}
		sceKernelDelayThread(50 * 1000);
	}
	if(sceNetApctlGetInfo(8, &info) != 0) {
		printf("Got an unknown IP\n");
		while(running) sceKernelDelayThread(50 * 1000);
		sceKernelExitGame();
	}
	printf("Connected, My IP is %s\n", info.ip);
}

void psp_wait(void) {
	while(running) sceKernelDelayThread(50 * 1000);
}

void psp_init(void) {
	int thid;

	pspDebugScreenInit();
	pspDebugScreenSetXY(0, 0);

	printf(FR_SERVER " for PSP bootstrap\n");

	thid = sceKernelCreateThread("update_thread", psp_callback_thread, 0x11, 0xfa0, 0, NULL);
	if(thid >= 0) {
		sceKernelStartThread(thid, 0, NULL);
	} else {
		printf("Failed to start thread\n");
		psp_wait();
		sceKernelExitGame();
	}
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	if(pspSdkInetInit()) {
		printf("Could not init the network\n");
		psp_wait();
		sceKernelExitGame();
	} else {
		printf("Network initialization successful\n");
	}
	if(sceWlanGetSwitchState() != 1) {
		printf("Turn the Wi-Fi switch on\n");
		while(sceWlanGetSwitchState() != 1) {
			sceKernelDelayThread(1000 * 1000);
		}
	} else {
		printf("Wi-Fi is turned on\n");
	}

	psp_net_init();
}

void psp_exit(int x) {
	sceKernelExitGame();
}
#endif
