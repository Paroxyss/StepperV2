// on document load
window.onload = function () {
    // wait for file from #fileInput 
    const fileInput = document.getElementById("fileInput")
    fileInput.addEventListener("change", function (e) {
        if (!fileInput.files.length) return;
        const file = fileInput.files[0];
        parseFile(file).then(data => {
            const buffer = makeBinary(parseMidiJson(data))
            // make download the buffer
            upload(buffer);
        })
    })
    function parseFile(file) {
        //read the file
        return new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = function (e) {
                const midi = new Midi(e.target.result);
                resolve(midi)
            };
            reader.readAsArrayBuffer(file);
        })
    }
    function parseMidiJson(input) {
        const tracks = input.tracks;
        let eventPile = []

        tracks.forEach((track) => {
            if (track.instrument.percussion) {
                return
            }
            track.notes.forEach((event) => {
                event.type = 1; // note on
                event.channel = track.channel;
                eventPile.push(event)
                // add note off event
                let event2 = Object.assign({}, event);
                event2.type = 0; // note off
                event2.ticks += event.durationTicks;
                eventPile.push(event2)
            })
        })

        // sort eventPile by time and off events first
        eventPile.sort((a, b) => {
            if (a.ticks < b.ticks) return -1;
            if (a.ticks > b.ticks) return 1;
            return a.type - b.type;
        })

        return ({ tempo: Math.round(input.header.tempos[0].bpm), timeDivision: input.header.ppq, pile: eventPile })
    }
    function download(content, mimeType, filename) {
        const a = document.createElement('a') // Create "a" element
        const blob = new Blob([content], { type: mimeType }) // Create a blob (file-like object)
        const url = URL.createObjectURL(blob) // Create an object URL from blob
        a.setAttribute('href', url) // Set "a" element link
        a.setAttribute('download', filename) // Set download filename
        a.click() // Start downloading
    }
}

