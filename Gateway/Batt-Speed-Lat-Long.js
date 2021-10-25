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
  var battery = out[0];
  var vmph = out[1];
  var vmps = out[2];
  var lat = out[3];
  var long = out[4];

  //Here I converted those strings into floats (long numbers)
  var vmphFloat = parseFloat(vmph);
  var vmpsFloat = parseFloat(vmps);
  var latFloat = parseFloat(lat);
  var longFloat = parseFloat(long);
  var data = {"battery" : battery};
  if(!Number.isNaN(latFloat))
  {
	data.latitude = latFloat;
  }
  if(!Number.isNaN(longFloat))
  {
	data.longitude = longFloat;
  }
    if(!Number.isNaN(vmphFloat))
  {
	data.vmph = vmphFloat;
  }
      if(!Number.isNaN(vmpsFloat))
  {
	data.vmps = vmpsFloat;
  }
  return data;
}
