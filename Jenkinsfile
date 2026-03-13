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
						sh("make -j4 package/install.exe")
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
						sh("./configure --prefix=ms0:/PSP/GAME/fhttpd --target=PSP --disable-ssl")
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
