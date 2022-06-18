
const { WebrtcTransportWrapper } = require('..');
const webrtc = new WebrtcTransportWrapper('sendonly', '127.0.0.1', 44444);

const response = webrtc.Init();


console.log(JSON.stringify(response));


setTimeout(function() {

}, 10000);