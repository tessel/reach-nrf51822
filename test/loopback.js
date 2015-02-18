var noble = require('noble');

// Use LE UUIDs
var service_uuid = "d32a1100d4194f52a41c9bc640116a45"
  , gossip_send = "d32a1101d4194f52a41c9bc640116a45"
  , gossip_recieve = "d32a1102d4194f52a41c9bc640116a45";
  
var outgoing
  , incoming;

var toSend = new Buffer([0x80, 0x20, 0x20]);

noble.startScanning();

noble.on('discover', function(peripheral){
  console.log('# Peripheral found:', peripheral.advertisement);
  if (peripheral.advertisement.localName == "Reach"){
    peripheral.connect();
    
    peripheral.on('connect', onConnect);
  }
});

function onConnect(){
  //this == peripheral
  console.log('# Reach connected');
  
  noble.stopScanning();
  this.on('disconnect', function(){
    noble.startScanning([service_uuid]);
  });
  
  this.discoverAllServicesAndCharacteristics(function(err, services, characteristics){
    if (err){
      console.log('# Discover error:', err);
    }
    for (var i=0; i<services.length; i++){
      if (services[i].uuid == service_uuid){
        console.log('# Gossip service found');

        for (var i=0; i<characteristics.length; i++){
          if (characteristics[i].uuid == gossip_send){

            console.log('# Outgoing identified:', characteristics[i].uuid);
            outgoing = characteristics[i];
            outgoing.on('write', onWrite);

          } else if (characteristics[i].uuid == gossip_recieve){

            console.log('# Incoming identified:', characteristics[i].uuid);
            incoming = characteristics[i];
            incoming.notify(true, function(err){
              if(err){
                console.log('FAILURE: Could not subscribe to notifications:', err);
                process.exit(1);
              }
            });
            incoming.on('read', onRead);

          };
        }

      }
    }
  });
};

function onRead(data, isNotification){
  //this == characteristic
  console.log('# Read', (isNotification ? 'Notification:' : ':'), data instanceof Buffer, data);
  if (data.equals(new Buffer([0x80, 0x80])) && isNotification){
    console.log('SUCCESS: Characteristic subscription acknowledged');
    outgoing.write(toSend, false);
  } else {
    if(data.equals(toSend)){
      console.log('SUCCESS: Response recieved');
      process.exit(0);
    }else{
      console.log('FAILURE: No response');
      process.exit(1);
    }
  }
};

function onWrite(){
  //this == characteristic
  console.log('# Write');
};
