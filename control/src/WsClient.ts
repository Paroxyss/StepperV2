import ws from 'ws';
import colors from "colors/safe"

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

export class WsClient {
    ws: ws;
    state: (-1 | { motor: number, channel: number })[] = new Array(120).fill(-1);
    s2 = Array.from(this.state)
    playing = false;
    transpose = 0;
    cbList: { [key: string | number]: { to: NodeJS.Timeout, cb: (data: any) => void } } = {};
    wstag = 0;
    constructor(public log: (text: string) => void) {
        this.ws = new ws('ws://192.168.1.99:81');
        this.ws.on("message", (data) => {
            if (data.toString()[0] != '{' && Buffer.isBuffer(data)) {
                this.handleMotors(data);
            }
            else {
                try {
                    const msg = JSON.parse(data.toString());
                    const obj = this.cbList[msg.tag];
                    if (obj) {
                        obj.cb(msg.data);
                        clearTimeout(obj.to);
                        delete this.cbList[msg.tag];
                    }
                }
                catch (e) {
                    console.log(e);
                }
            }
        })
        this.ws.on("open", () => {
            this.send("motorStream" , true);
        })
    }
    handleMotors(data: Buffer) {
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
                const index = this.state.findIndex(e => e != -1 && e.motor == event.motorId);

                if (index != -1) {
                    this.state[index] = -1;
                }
                if (event.state) {
                    this.state[event.note] = { motor: event.motorId, channel: event.channel };
                    this.s2[event.note] = { motor: event.motorId, channel: event.channel };
                }
            }
        }
    }
    send<T>(type: string, data?: any, cb?: (data: T) => void) {
        let obj: any = {
            tag: this.wstag,
            type: type,
        }
        if (data !== undefined) {
            obj["data"] = data;
        }
        this.ws.send(JSON.stringify(obj))
        if (cb) {
            this.cbList[this.wstag] = {
                to: setTimeout(() => {
                    this.log(`timeout ${type}`)
                    delete this.cbList[this.wstag];
                }, 10000),
                cb: cb
            }
        }
        this.wstag = (this.wstag + 1) % 4096;
    }
    resume() {
        this.send("resume");
        this.playing = true;
    }
    pause() {
        this.send("pause");
        this.playing = false;
    }
    setTranspose(transpose: number) {
        this.send("setTranspose", transpose);
    }
    listFiles() {
        return new Promise<string[]>(res => {
            this.send("ls", undefined, (data) => {
                res(data as any);
            })
        })
    }
    request(song: number) {
        this.send("request", song);
        this.playing = true;
    }
    delete(song: number) {
        this.send("delete", song);
    }
    getMotorLine() {
        const str = this.s2.map(id => id == -1 ? ' ' : colorFuncs[id.channel % colorFuncs.length](id.motor.toString())).join('');
        this.s2 = Array.from(this.state)
        return str;
    }

}