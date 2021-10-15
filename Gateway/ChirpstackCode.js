//Shown below is the updated custom Javascript code I wrote on chirpstack for GPS
//With some added comments for explanation

function Decode(fPort, bytes, variables) {

  //Here I loop through the array of bytes created by the Data packet from the
  //node and add each one to the "result" which is initialized as a string.
  //I use the charcode to translate into something that will actually fit as a string.  
  var result = "";
  for (var i = 0; i < bytes.length; i++) 
  {
    result += String.fromCharCode(parseInt(bytes[i]));
  }

  //Here I split our string up into an array of strings anywhere
  //that there was a comma. Then save those strings into lat and long
  var out = result.split(",");
  var lat = out[0];
  var long = out[1];

  //Here I converted those strings into floats (long numbers)
  var latFloat = parseFloat(lat);
  var longFloat = parseFloat(long);

  //Then push those floats labeled as lat and long
  var data = {"latitude" : latFloat, "longitude" : longFloat}
  return data;
}
