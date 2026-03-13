pipeline {
	agent {
		label "built-in"
	}
	stages {
		stage("Build") {
			parallel {
				stage("Build for Linux 64-bit") {
					agent {
						label "built-in"
					}
					steps {
						sh("make distclean")
						sh("./configure")
						sh("make -j4")
					}
				}
				stage("Build for Windows 32-bit") {
					agent {
						label "built-in"
					}
					environment {
						WATCOM = "/usr/watcom"
						INCLUDE = "/usr/watcom/h:/usr/watcom/h/nt"
						PATH = "/usr/watcom/binl64:${env.PATH}"
					}
					steps {
						sh("make distclean")
						sh("./configure --prefix=C:/Feather --target=Watcom --disable-ssl")
						sh("make -j4 server/install.exe")
						sh("mv server/install.exe install-win32.exe")
						archiveArtifacts("install-win32.exe")
					}
				}
				stage("Build for PSP") {
					agent {
						label "built-in"
					}
					environment {
						PSPDEV = "/usr/pspdev"
						PATH = "/usr/pspdev/bin:${env.PATH}"
					}
					steps {
						sh("make distclean")
						sh("./configure --prefix=ms0:/PSP/GAME/fhttpd --target=PSP --disable-ssl")
						sh("make -j4 server/fhttpd.pbp")
						sh("rm -rf tmp")
						sh("make install DESTDIR=`pwd`/tmp/")
						sh("rm -rf tmp/ms0:/PSP/GAME/fhttpd/bin")
						sh("mv server/fhttpd.pbp tmp/ms0:/PSP/GAME/fhttpd/EBOOT.PBP")
						sh("sed -i 's%/usr/fhttpd%ms0:/PSP/GAME/fhttpd%g' tmp/ms0:/PSP/GAME/fhttpd/etc/fhttpd/fhttpd.conf")
						sh("cd tmp/ms0:/PSP/GAME/ && zip -rv ../../../../fhttpd-psp.zip .")
						sh("rm -rf tmp")
						archiveArtifacts("fhttpd-psp.zip")
					}
				}
			}
			post {
				always {
					notifyDiscord()
				}
			}
		}
	}
}
