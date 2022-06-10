import fs from 'fs';
import ws from "ws";
import md5 from "md5-file"
// open file as buffer

const filePath = process.argv[2];

const file = fs.readFileSync(filePath);

function createChunks(file: Buffer, cSize: number = 1024): Buffer[] {
    let startPointer = 0;
    let endPointer = file.length;
    let chunks = [];
    while (startPointer < endPointer) {
        let newStartPointer = startPointer + cSize;
        chunks.push(file.slice(startPointer, newStartPointer));
        startPointer = newStartPointer;
    }
    return chunks;
}

const chunks = createChunks(file);
console.log(chunks.length);
const hash = md5.sync(filePath);

// open websocket
const mainWs = new ws("ws://192.168.1.99:81");
mainWs.on("open", () => {
    mainWs.on("message", (message) => {
        try {
            const msg = JSON.parse(message.toString());
            if (msg.type == "upload") {
                if (msg.data == "ok") {
                    // connect to the upload socket
                    console.log("WS - 2 - opening");

                    const secondWs = new ws("ws://192.168.1.99:82");
                    secondWs.on("open", () => {
                        console.log("WS - 2 - opened");
                        secondWs.on("message", (message) => {
                            console.log("WS - 2 - ", message.toString());
                            try {
                                const msg = JSON.parse(message.toString());
                                if (msg.type == "ready") {
                                    console.log("WS - 2 - sending chunks");
                                    (async () => {
                                        for (let i = 0; i < chunks.length; i++) {
                                            await new Promise<void>((resolve) => {
                                                secondWs.send(chunks[i], (err) => {
                                                    if (err) {
                                                        console.log("WS - 2 - error sending chunk", err);
                                                    } else {
                                                        console.log("WS - 2 - chunk sent");
                                                        setTimeout(() => {
                                                            resolve()
                                                        }, 50);
                                                    }
                                                });
                                            });
                                        }
                                    })();
                                }
                            }
                            catch (e) {
                                console.log("WS - 2 - ", e);
                            }
                        })
                        const obj = { "type": "uploadInfo", "fileHash": hash, "fileSize": file.length }
                        console.log(obj);
                        
                        secondWs.send(JSON.stringify(obj))
                    })
                }
            }
        }
        catch (e) {
            //console.log(e);
        }
    });
    mainWs.on("close", () => {
        console.log("closed");
    });
    mainWs.send(JSON.stringify({ type: "upload" }))
})
