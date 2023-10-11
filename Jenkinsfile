@Library('xmos_jenkins_shared_library@v0.27.0')

def runningOn(machine) {
    println "Stage running on:"
    println machine
}

getApproval()
pipeline {
    agent none

    options {
        timestamps()
        buildDiscarder(xmosDiscardBuildSettings(onlyArtifacts=false))
    } // options

    stages {
        stage ('Create release package') {
            agent {
                label 'linux&&x86_64'
            }
            stages {
                stage ('Build release package') {
                    steps {
                        runningOn(env.NODE_NAME)
                        // fetch submodules
                        sh 'git submodule update --init --jobs 4'
                        // Run release script
                        sh 'tools/ci/release.sh'
                        // archive release package
                        archiveArtifacts artifacts: 'host_xvf_control_release_*.zip', fingerprint: true
                        stash name: "release_source", includes: "release/**"
                    }
                }
            } // stages
            post {
                cleanup {
                    xcoreCleanSandbox()
                }
            }
        } // Create release package

        stage ('Cross-platform Builds & Tests') {
            parallel {

                stage ('RPI Build & Test') {
                    agent {
                        label 'armv7l&&raspian'
                    }
                    stages {
                        stage ('Build') {
                            steps {
                                runningOn(env.NODE_NAME)
                                // unstash release files
                                unstash "release_source"
                                // copy test folder into release folder
                                sh 'mv test release/'
                                // build
                                dir('release/build') {
                                    sh 'cmake -S .. -DTESTING=ON && make -j4'
                                    // archive RPI binaries
                                    sh 'mkdir rpi && cp xvf_host *.so rpi/'
                                    archiveArtifacts artifacts: 'rpi/*', fingerprint: true
                                }
                                dir('release/fwk_rtos/modules/sw_services/device_control/api') {
                                    archiveArtifacts artifacts: 'device_control_shared.h', fingerprint: true
                                }
                            }
                        }
                        stage ('Create Python enviroment') {
                            steps {
                                //sh 'python3 -m venv .venv && source .venv/bin/activate && pip3 install -r requirements-dev.txt'
                                sh 'python3 -m venv .venv && source .venv/bin/activate && pip install pytest && pip install jinja2'
                            }
                        }
                        stage ('Test') {
                            steps {
                                dir('release/test') {
                                    sh 'source ../../.venv/bin/activate && pytest -s'
                                }
                            }
                        }
                    } // stages
                    post {
                        cleanup {
                             xcoreCleanSandbox()
                        }
                    }
                } // RPI Build & Test

                stage ('Linux x86 Build & Test') {
                    agent {
                        label 'linux&&x86_64'
                    }
                    stages {
                        stage ('Build') {
                            steps {
                                runningOn(env.NODE_NAME)
                                // unstash release files
                                unstash "release_source"
                                // copy test folder into release folder
                                sh 'mv test release/'
                                // build
                                dir('release/build') {
                                    sh 'cmake -S .. -DTESTING=ON && make -j4'
                                    // archive RPI binaries
                                    sh 'mkdir linux_x86 && cp xvf_host *.so linux_x86/'
                                    archiveArtifacts artifacts: 'linux_x86/*', fingerprint: true
                                }
                            }
                        }
                        stage ('Create Python enviroment') {
                            steps {
                                createVenv("requirements.txt")
                                withVenv{
                                    sh 'pip install -r requirements.txt'
                                }
                            }
                        }
                        stage ('Test') {
                            steps {
                                withVenv{
                                    dir('release/test') {
                                        sh 'pytest -s'
                                    }
                                }
                            }
                        }
                    } // stages
                    post {
                        cleanup {
                             xcoreCleanSandbox()
                        }
                    }
                } // Linux x86 Build & Test

                stage ('Mac x86 Build & Test') {
                    agent {
                        label 'macos&&x86_64'
                    }
                    stages {
                        stage ('Build') {
                            steps {
                                runningOn(env.NODE_NAME)
                                // unstash release files
                                unstash "release_source"
                                // copy test folder into release folder
                                sh 'mv test release/'
                                // build
                                dir('release/build') {
                                    sh 'cmake -S .. -DTESTING=ON && make -j4'
                                    // archive Mac binaries
                                    sh 'mkdir mac_x86 && cp xvf_host *.dylib mac_x86/'
                                    archiveArtifacts artifacts: 'mac_x86/*', fingerprint: true
                                }
                            }
                        }
                        stage ('Create Python enviroment') {
                            steps {
                                createVenv("requirements.txt")
                                withVenv{
                                    sh 'pip install -r requirements.txt'
                                }
                            }
                        }
                        stage ('Test') {
                            steps {
                                withVenv{
                                    dir('release/test') {
                                        sh 'pytest -s'
                                    }
                                }
                            }
                        }
                    } // stages
                    post {
                        cleanup {
                             xcoreCleanSandbox()
                        }
                    }
                } // Mac x86 Build & Test

                stage ('Mac arm64 Build & Test') {
                    agent {
                        label 'macos&&arm64'
                    }
                    stages {
                        stage ('Build') {
                            steps {
                                runningOn(env.NODE_NAME)
                                // unstash release files
                                unstash "release_source"
                                // copy test folder into release folder
                                sh 'mv test release/'
                                // build
                                dir('release/build') {
                                    sh 'cmake -S .. -DTESTING=ON && make -j4'
                                    // archive Mac binaries
                                    sh 'mkdir mac_arm64 && cp xvf_host *.dylib mac_arm64/'
                                    archiveArtifacts artifacts: 'mac_arm64/*', fingerprint: true
                                }
                            }
                        }
                        stage ('Create Python enviroment') {
                            steps {
                                createVenv("requirements.txt")
                                withVenv{
                                    sh 'pip install -r requirements.txt'
                                }
                            }
                        }
                        stage ('Test') {
                            steps {
                                withVenv{
                                    dir('release/test') {
                                        sh 'pytest -s'
                                    }
                                }
                            }
                        }
                    } // stages
                    post {
                        cleanup {
                             xcoreCleanSandbox()
                        }
                    }
                } // Mac arm64 Build & Test

                stage ('Windows Build & Test') {
                    agent {
                        label 'sw-bld-win0'
                    }
                    stages {
                        stage ('Build') {
                            steps {
                                runningOn(env.NODE_NAME)
                                // unstash release files
                                unstash "release_source"
                                // copy test folder into release folder
                                bat 'move test release/'
                                // build
                                dir('release/build') {
                                    withVS('vcvars32.bat') {
                                        bat 'cmake -G "Ninja" -S .. -DTESTING=ON'
                                        bat 'ninja'
                                    }
                                    // archive Mac binaries
                                    bat 'mkdir windows && cp xvf_host.exe *.dll windows/'
                                    archiveArtifacts artifacts: 'windows/*', fingerprint: true
                                }
                            }
                        }
                        stage ('Create Python enviroment') {
                            steps {
                                createVenv("requirements.txt")
                                withVenv{
                                    bat 'pip install -r requirements.txt'
                                }
                            }
                        }
                        stage ('Test') {
                            steps {
                                withVenv{
                                    dir('release/test') {
                                        bat 'pytest -s'
                                    }
                                }
                            }
                        }
                    } // stages
                    post {
                        cleanup {
                             xcoreCleanSandbox()
                        }
                    }
                } // Windows Build & Test
            } // parallel
        } // Cross-platform Builds & Tests
    } // stages
} // pipeline
