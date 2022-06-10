import * as blessed from 'blessed';
import { WsClient } from './WsClient';

const socket = new WsClient(log)

const screen = blessed.screen();
const mainBox = blessed.box({
    top: 'center',
    left: 'center',
    width: '100%',
    height: '100%',
    style: {
        bg: 'black'
    }
});
screen.append(mainBox);
const musicLive = blessed.text({
    parent: mainBox,
    mouse: true,
    keys: true,
    width: '70%',
    height: '100%',
    scrollable: false,
    left: 0,
    top: 0,
    name: 'logs',
    border: "line",
})
const menuBox = blessed.box({
    parent: mainBox,
    mouse: true,
    keys: true,
    width: '30%',
    height: '100%',
    scrollable: false,
    left: '70%',
    top: 0,
    name: 'menu',
    border: "line",
    bg: 'grey'
})

const resumeButton = blessed.button({
    parent: menuBox,
    mouse: true,
    keys: true,
    width: '40%',
    height: "5%",
    left: "0",
    top : 0,
    name: "resume",
    content : "Resume",
    align: "center",
    style: {
        bg: 'white',
        fg : "black",
    }
})

const pauseButton = blessed.button({
    parent: menuBox,
    mouse: true,
    keys: true,
    width: '40%',
    height: "5%",
    left: "40%+1",
    top : 0,
    name: "pause",
    content : "Pause",
    align: "center",
    style: {
        bg: 'white',
        fg : "black",
        hover : {
            bg: 'lightgrey',
        }
    }
})

const musicList = blessed.list({
    parent: menuBox,
    mouse: true,
    keys: true,
    width: '90%',
    height: "1",
    left: "0",
    top : "10%",
    name: "musicList",
    align: "left",
    items: [
        "Fetching music..."
    ],
    bg:"grey",
    style: {
        selected: {
            bg: 'lightgrey',
            fg: 'black',
        },
        item : {
            bg: 'grey',
        }
    }
})


pauseButton.on('press', () => {
    socket.pause()
})
resumeButton.on('press', () => {
    socket.resume()
})
socket.ws.on("open", () => {
    socket.listFiles().then(files => {
        log(`opened with \n${files.join("\n")}`)
        musicList.setItems(files as any[]);
        musicList.height = files.length;
        screen.render()
    })
})
function log (text: string) {
    musicLive.unshiftLine(text)
    screen.render()
}

musicList.on("select", (item, index) => {
    socket.request(index)
})


setInterval(() => {
    if(socket.playing){
        musicLive.unshiftLine(socket.getMotorLine())
        screen.render()
    }
}, 75);

screen.key(['q', "C-c"], function () {
    process.exit(0);
});
let i = 0

screen.render();