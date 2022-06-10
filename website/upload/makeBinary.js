function makeBinary(data) {
    // data is an array of events
    // each event is an object with keys:
    // time, channel, percu, note, velocity
    // time is in beats

    const pile = data.pile;
    //create arraybuffer
    var buffer = new ArrayBuffer(pile.length * 7 + 21); // 7 bytes per event, 21 bytes for header
    var view = new Uint8Array(buffer);
    // ask user for 9 char name
    var name = prompt("Please enter a name for your song:")
    if (name.length > 15) {
        name = name.slice(0, 15)
    }
    // check if name is valid (all chars are alphanumeric and no spaces)
    if (!name.match(/^[a-zA-Z0-9 ]+$/)) {
        alert("Invalid name. Please try again.")
        throw new Error("Invalid name")
    }
    // write name to buffer
    for (var i = 0; i < name.length; i++) {
        view[i] = name.charCodeAt(i);
    }
    // write tempo to buffer
    view[15+1] = data.tempo;
    // write time division to buffer
    writeUInt32(view, 15+2, data.timeDivision)
    // write events to buffer

    var offset = 21;
    for (var i = 0; i < pile.length; i++) {
        writeEvent(view, offset + i * 7, pile[i], pile[i + 1] ? pile[i + 1].ticks - pile[i].ticks : 0);
    }
    return buffer;

}

function writeEvent(view, offset, event, delta) {
    // write event to buffer
    view[offset] = event.type
    view[offset + 1] = event.channel
    view[offset + 2] = event.midi
    writeUInt32(view, offset + 3, delta)
}

function writeUInt32(view, offset, value) {
    // the view is an Uint8Array, so we have to split value into bytes, for little endian
    if(value > 0xffffffff) throw new Error("Value too large")
    view[offset] = value & 0xff
    view[offset + 1] = (value >>> 8) & 0xff
    view[offset + 2] = (value >>> 16) & 0xff
    view[offset + 3] = (value >>> 24) & 0xff
}
