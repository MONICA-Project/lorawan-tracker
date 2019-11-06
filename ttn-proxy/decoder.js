function Decoder(bytes) {
  var decoded = {};
  coord_scale = 10000000;
  // 32 bit, latitude
  lat = ((0xff & bytes[0]) << 24) | ((0xff & bytes[1]) << 16) | ((0xff & bytes[2]) << 8) | (0xff & bytes[3] << 0);
  // 32 bit, longitude
  lon = ((0xff & bytes[4]) << 24) | ((0xff & bytes[5]) << 16) | ((0xff & bytes[6]) << 8) | (0xff & bytes[7] << 0);
  // 16 bit, altitude/height
  alt = ((0xff & bytes[8]) <<  8) | ((0xff & bytes[9]) << 0);
  //  8 bit, number of GPS sat
  sat = (0xff & bytes[10]);
  // coordinates need to be converted from int32 to float
  decoded['lat'] = lat / coord_scale;
  decoded['lon'] = lon / coord_scale;
  // altitude/height is in meters
  decoded['alt'] = alt;
  decoded['sat'] = sat;
  return decoded;
}


