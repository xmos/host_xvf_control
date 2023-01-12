@Library('xmos_jenkins_shared_library@v0.21.0')

def runningOn(machine) {
    println "Stage running on:"
    println machine
}

getApproval()
pipeline {
    agent none
    stages {
        stage('RPI Build & Test') {
            agent {
                label 'armv7l&&raspian'
            }
            stages {
                stage ('Build') {
                    steps {
                        runningOn(env.NODE_NAME)
                        // fetch submodules
                        sh 'git submodule update --init --jobs 4'
                        // build
                        dir('build') {
                            sh 'cmake -S .. -DTESTING=ON && make -j4'
                            // archive RPI binaries
                            archiveArtifacts artifacts: 'xvf_host, libdevice_*', fingerprint: true
                        }
                    }
                }
                stage ('Create Python enviroment') {
                    steps {
                        //sh 'python3 -m venv .venv && source .venv/bin/activate && pip3 install -r requirements-dev.txt'
                        sh 'python3 -m venv .venv && source .venv/bin/activate && pip3 install pytest'
                    }
                }
                stage ('Test') {
                    steps {
                        dir('test') {
                            sh 'source ../.venv/bin/activate && pytest -s'
                        }
                    }
                }
            } // stages
            post {
                cleanup {
                    cleanWs()
                }
            }
        } // RPI build & test
    } // stages
} // pipeline
