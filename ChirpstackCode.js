//Shown below is the actual custom Javascript code I wrote on chirpstack
//With some added comments for explanation
//This is in just a .txt as there isn't a way to download off of chirpstack

// Decode decodes an array of bytes into an object.
//  - fPort contains the LoRaWAN fPort number
//  - bytes is an array of bytes, e.g. [225, 230, 255, 0]
//  - variables contains the device variables e.g. {"calibration": "3.5"} (both the key / value are of type string)
// The function must return an object, e.g. {"temperature": 22.5}

function Decode(fPort, bytes, variables) {

    //Here we loop through the array of bytes created by the Data packet from the
    //node and add each one to the "result" which is initialized as a string.
    //We use our charcode to translate into something that will actually fit as a string.
    var result = "";
    for (var i = 0; i < bytes.length; i++) {
        result += String.fromCharCode(parseInt(bytes[i]));
    }

    //Here we also converted our array of bytes into a string to show how this array
    //Is translated, I also used it for testing and figuring a lot of this stuff out
    var ogmessage = bytes.toString();

    //We initialize a javascript object "data" and give it both of our strings to 
    //then add to the json object on the incoming packets from the node
    var data = { "Message": result, "ogMessage": ogmessage }
    return data;
}