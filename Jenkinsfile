@Library('xmos_jenkins_shared_library@v0.21.0')

def runningOn(machine) {
    println "Stage running on:"
    println machine
}

getApproval()
pipeline {
    agent none
    stages {
        stage('RPI Build') {
            agent {
                label 'armv7l&&raspian'
            }
            steps {
                runningOn(env.NODE_NAME)
                // checkout repo
                checkout scm
                // fetch submodules
                sh 'git submodule update --init --jobs 4'
                // build
                dir('build') {
                    sh 'cmake -S .. && make'
                    dir('src') {
                        // archive RPI binaries
                        archiveArtifacts artifacts: 'xvf_host_rpi_*', fingerprint: true
                    }
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
