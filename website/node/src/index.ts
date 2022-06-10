import WebSocket from "ws";

for (let i = 1; i < 255; i++) {
    try {
        const ws = new WebSocket(`ws://192.168.1.${i}:81`);
        ws.on("message", (data) => {
            console.log(i, data.toString());
        })
        ws.on("error", err => {})
    } catch (e) {
    }
}