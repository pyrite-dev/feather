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
			}
			post {
				always {
					notifyDiscord()
				}
			}
		}
	}
}
