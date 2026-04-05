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
						sh("./configure --enable-mods-shared=all")
						sh("make -j4")
					}
				}
				stage("Build for Windows 32-bit (Legacy)") {
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
						sh("./configure --prefix=C:/Feather --target=Watcom --disable-ssl --enable-mods-shared=all")
						sh("make -j4 package/install.exe")
						sh("mv package/install.exe install-win32-legacy.exe")
						archiveArtifacts("install-win32-legacy.exe")
					}
				}
				stage("Build for Windows 32-bit") {
					agent {
						label "built-in"
					}
					steps {
						sh("make distclean")
						sh("rm -rf openssl && git clone https://github.com/clamwin/openssl --depth=1")
						sh("./configure --prefix=C:/Feather --target=Windows --enable-mods-shared=all")
						sh("echo CFLAGS+=-I`pwd`/openssl/include >> config.mk")
						sh("echo LDFLAGS+=-L`pwd`/openssl/lib/mingw/x86 >> config.mk")
						sh("make CC=i686-w64-mingw32-gcc AR=i686-w64-mingw32-ar RC=i686-w64-mingw32-windres -j4 package/install.exe")
						sh("mv package/install.exe install-win32.exe")
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
						sh("./configure --prefix=ms0:/PSP/GAME/fhttpd --target=PSP --disable-ssl --enable-mods-static=all")
						sh("make -j4 package/fhttpd-psp.zip")
						sh("mv package/fhttpd-psp.zip ./")
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
