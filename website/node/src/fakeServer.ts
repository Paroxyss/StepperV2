import WebSocket from "ws";

const server = new WebSocket.Server({ port: 8021 });
server.on("connection", (ws) => {
    console.log("connection");
    ws.send("Hello from stepper motor fake server")
    ws.on("message", (data) => {
        console.log(data.toString());
        
        const obj = JSON.parse(data.toString());
        let res : any = {tag : obj.tag};
        switch(obj.type){
            case "ls":
                res.data = [
                    "Billie Jean",
                    "Beat It",
                    "Thriller",
                ]
                break;
            case "resume":
            case "pause":
            case "request":
                break;
            case "udp":
                res.data = 8022;
                break;
            default:
                return;
        }
	console.log(res)
        ws.send(JSON.stringify(res));

    });
    ws.on("error", err => {throw err});
})
