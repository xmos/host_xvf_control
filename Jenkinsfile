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
            steps {
                runningOn(env.NODE_NAME)
                // fetch submodules
                sh 'git submodule update --init --jobs 4'
                // build
                dir('build') {
                    sh 'cmake -S .. -DTESTING=ON && make'
                    // archive RPI binaries
                    archiveArtifacts artifacts: 'xvf_hostapp_rpi, libdevice_*', fingerprint: true
                }
                dir('test') {
                    sh 'python3 -m venv .venv && source .venv/bin/activate && pip install -r requirements.txt'
                    sh 'pytest -s'
                }
            }
            post {
                cleanup {
                    cleanWs()
                }
            }
        } // RPI build
    } // stages
} // pipeline
