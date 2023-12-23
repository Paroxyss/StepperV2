// open file as buffer


function createChunks(file, cSize = 1024) {
    let startPointer = 0;
    let endPointer = file.byteLength;
    let chunks = [];
    while (startPointer < endPointer) {
        let newStartPointer = startPointer + cSize;
        chunks.push(file.slice(startPointer, newStartPointer));
        startPointer = newStartPointer;
    }
    return chunks;
}

function upload(buffer) {
    const ip = document.getElementById('ip').value;
    const progressBar = document.getElementById('progress');
    console.log(buffer);
    const chunks = createChunks(buffer);
    console.log(chunks.length);
    const hash = md5(buffer);

    // open websocket
    const mainWs = new WebSocket(`ws://${ip}:81`);
    mainWs.onopen = () => {
        console.log("websocket opened");
        mainWs.onmessage = (message) => {
            console.log(message.data.toString());
            try {
                const msg = JSON.parse(message.data.toString());
                if (msg.data == "ok") {
                    // connect to the upload socket
                    console.log("WS - 2 - opening");

                    const secondWs = new WebSocket(`ws://${ip}:82`);
                    secondWs.onopen = () => {
                        console.log("WS - 2 - opened");
                        secondWs.onmessage = (message) => {
                            console.log("WS - 2 - ", message.data.toString());
                            try {
                                const msg = JSON.parse(message.data.toString());
                                if (msg.type == "ready") {
                                    console.log("WS - 2 - sending chunks");
                                    (async () => {
                                        console.log("start");
                                        for (let i = 0; i < chunks.length; i++) {
                                            await new Promise((resolve) => {
                                                console.log("sending chunk", i);
                                                secondWs.send(chunks[i]);
                                                console.log("WS - 2 - chunk sent");
                                                // increment value
                                                progressBar.value = i / chunks.length * 100;
                                                if (i == chunks.length - 1) {
                                                    progressBar.classList.add('progress-bar-waiting');
                                                    progressBar.value = 100;
                                                }
                                                setTimeout(() => {
                                                    resolve()
                                                }, 25);
                                            });
                                        }

                                    })();
                                }
                                else if (msg.type == "uploadSuccess") {
                                    console.log("WS - 2 - upload success");
                                    // make progress bar green
                                    progressBar.classList.remove('progress-bar-waiting');
                                    progressBar.classList.add('progress-bar-success');
                                    setTimeout(() => {
                                        progressBar.value = 0;
                                        progressBar.classList.remove('progress-bar-success');
                                    }, 1000);
                                    secondWs.close();
                                    mainWs.close();
                                }
                                else if (msg.type == "uploadError") {
                                    console.log("WS - 2 - upload error");
                                    alert("Erreur lors de l'upload : " + msg.error);
                                    secondWs.close();
                                    mainWs.close();
                                }
                            }
                            catch (e) {
                                console.log("WS - 2 - ", e);
                            }
                        }
                        const obj = { "type": "uploadInfo", "fileHash": hash, "fileSize": buffer.byteLength }
                        console.log(obj);

                        secondWs.send(JSON.stringify(obj))
                    }
                }
            }
            catch (e) {
                //console.log(e);
            }
        };
        mainWs.onclose = () => {
            console.log("closed");
        };
        mainWs.send(JSON.stringify({ type: "upload" }))
    }

}