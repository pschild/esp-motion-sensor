import groovy.json.JsonSlurper

def loadESPDefinitions() {
    def jsonSlurper = new JsonSlurper()
    URL jsonUrl = new URL("https://raw.githubusercontent.com/pschild/esp-jenkins-config/master/esp-config.json")
    def jsonResponse = jsonSlurper.parse(jsonUrl)
    
    def result = []
    jsonResponse.each {
        result << it.chipId + "|" + it.pioEnv
    }
    
    return result
}

pipeline {
    agent any
    
    parameters {
        choice(name: 'ESP', choices: loadESPDefinitions(), description: 'Choose Target ESP')
    }
    
    environment {
        FIRMWARE_VERSION = "v${BUILD_NUMBER}-${sh(script:'git rev-parse HEAD', returnStdout: true).trim().take(7)}"
        CHIPID="${sh(script:'echo $ESP | cut -d"|" -f1', returnStdout: true).trim()}"
        PIOENV="${sh(script:'echo $ESP | cut -d"|" -f2', returnStdout: true).trim()}"
    }
    
    stages {
        stage('Debug') {
            steps {
                sh 'printenv'
            }
        }
        stage('Build Binary') {
            steps {
                withCredentials([usernamePassword(credentialsId: '4ba76353-3bab-4d0d-9364-9f9e9909495f', passwordVariable: 'WIFI_PASS', usernameVariable: 'WIFI_SSID')]) {
                    sh '''
                        pio run -t clean -e ${PIOENV}
                        pio run -e ${PIOENV} -v
                    '''
                }
            }
        }
        stage('Copy Binary') {
            steps {
                sh '''
                    FILENAME=.pio/build/${PIOENV}/firmware.bin
                    TARGETNAME=/var/binfiles/${CHIPID}/firmware-${FIRMWARE_VERSION}.bin
                    mkdir -p /var/binfiles/${CHIPID}
                    mv ${FILENAME} ${TARGETNAME}
                '''
            }
        }
    }
}
