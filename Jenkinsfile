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
			}
			post {
				always {
					notifyDiscord()
				}
			}
		}
	}
}
