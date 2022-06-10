import ws from 'ws';
// import keypress and set it up
import prompts from 'prompts';
import { Readable, Writable } from 'stream';
var keypress = require('keypress');
import colors from "colors/safe"

keypress(process.stdin);
const colorFuncs = [
    colors.red,
    colors.green,
    colors.yellow,
    colors.blue,
    colors.magenta,
    colors.cyan,
    colors.white,
    colors.gray,
]
// client to 192.168.1.99:81
const wsClient = new ws('ws://192.168.1.99:81');
let state : (-1 | {motor : number, channel : number})[] = new Array(120).fill(-1);
let s2 = Array.from(state)
let playing = false;
let transpose = 0;
let cbList: { [key: string | number]: { to: NodeJS.Timeout, cb: (data: any) => void } } = {};

// create readable stream
const rstream = new Readable({
    read(size) { }
})

wsClient.on('open', () => {
    console.log('connected');
    wsClient.send(JSON.stringify({ type: "motorStream", "data": true })) // query for motor stream
    process.stdin.on('keypress', async function (ch, key) {
        rstream.push(key.sequence);
        if (key && key.ctrl && key.name == 'c') {
            process.stdin.pause();
            stop()
        }
        switch (key.name) {
            case 'p':
                wsClient.send(JSON.stringify({ type: "pause" }))
                console.log('pause');
                playing = false;
                break;
            case 'r':
                wsClient.send(JSON.stringify({ type: "resume" }))
                console.log('resume');
                playing = true;
                break;
            case 'left':
                if (!playing) break;
                transpose -= 1;
                wsClient.send(JSON.stringify({ type: "transpose", "data": transpose }))
                break;
            case 'right':
                if (!playing) break;
                transpose += 1;
                wsClient.send(JSON.stringify({ type: "transpose", "data": transpose }))
                break;
            case 'down':
                if (!playing) break;
                transpose = 0;
                wsClient.send(JSON.stringify({ type: "transpose", "data": transpose }))
                break;
            case "s":
                wsClient.send(JSON.stringify({ type: "pause" }))
                playing = false;
                const tag = randint(0, 2048);
                wsClient.send(JSON.stringify({ tag: tag, type: "ls" }))
                cbList[tag] = {
                    to: setTimeout(() => {
                        console.log("timeout");
                    }, 5000), cb: (songNameArray: string[]) => {
                        songNameArray.unshift("Cancel");
                        // prompt for a string of songNameArray, return the id
                        prompts({
                            type: 'select',
                            name: 'songName',
                            message: 'Enter a song name',
                            choices: songNameArray.map((songName, i) => ({ title: songName, value: i })),
                            stdin: rstream
                        }).then(async (ans) => {
                            if (ans) {
                                ans.songName-=1;
                                if (ans.songName == -1) {
                                    return
                                }
                                playing = true;
                                wsClient.send(JSON.stringify({ type: "request", "data": ans.songName }))
                            }
                        })
                    }
                }
                break;
            case "d":
                wsClient.send(JSON.stringify({ type: "pause" }))
                const playingTmp = playing;
                playing = false;
                const tag2 = randint(0, 2048);
                wsClient.send(JSON.stringify({ tag: tag2, type: "ls" }))
                cbList[tag2] = {
                    to: setTimeout(() => {
                        console.log("timeout");
                    }, 5000), cb: (songNameArray: string[]) => {
                        songNameArray.unshift("Cancel");
                        // prompt for a string of songNameArray, return the id
                        prompts({
                            type: 'select',
                            name: 'songName',
                            message: 'Enter a song name to delete',
                            choices: songNameArray.map((songName, i) => ({ title: songName, value: i })),
                            stdin: rstream
                        }).then(async (ans) => {
                            if (ans) {
                                playing = playingTmp;
                                ans.songName -= 1
                                if (ans.songName != -1) {
                                    wsClient.send(JSON.stringify({ type: "delete", "data": ans.songName }))
                                }
                            }
                        })
                    }
                }
                break;
        }

    });
});
wsClient.on("message", (data) => {
    if (data.toString()[0] != '{' && Buffer.isBuffer(data)) {
        if (data.length % 4 == 0) {
            for (let i = 0; i < data.length / 4; i++) {
                // read 4 bytes
                const buf = new Uint8Array(data).slice(i * 4, i * 4 + 4);

                const event = {
                    motorId: buf[0],
                    state: buf[1],
                    note: buf[2],
                    channel: buf[3]
                }
                if (event.motorId == 255) return;
                const index = state.findIndex(e => e != -1 && e.motor == event.motorId);

                if (index != -1) {
                    state[index] = -1;
                }
                if (event.state) {
                    state[event.note] = { motor: event.motorId, channel: event.channel };
                    s2[event.note] = { motor: event.motorId, channel: event.channel };
                }
            }
        }
    }
    else {
        try {
            const msg = JSON.parse(data.toString());
            const obj = cbList[msg.tag];
            if (obj) {
                obj.cb(msg.data);
                clearTimeout(obj.to);
                delete cbList[msg.tag];
            }
        }
        catch (e) {
            console.log(e);
        }
    }
})
setInterval(() => {
    if (!playing) return
    console.log(s2.map(id => id == -1 ? ' ' : colorFuncs[id.channel % colorFuncs.length](id.motor.toString()) ).join(''));
    s2 = Array.from(state)
}, 25)




// intercept ctrl+c for stop motor stream
process.on('SIGINT', () => {
    stop()
})

process.stdin.setRawMode(true);
process.stdin.resume();

function stop() {
    console.log('stopping motor stream');

    wsClient.send(JSON.stringify({ type: "motorStream", "data": false }), () => {
        console.log('stopped');
        process.exit(0);
    })
}

// randint a to b
function randint(a: number, b: number) {
    return Math.floor(Math.random() * (b - a + 1) + a);
}