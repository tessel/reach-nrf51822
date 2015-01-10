var noble = require('noble');

var service_uuid = "d752c5fb13804cd5b0efcac7d72cff20"
  , gossip_send = "883f1e6b76f64da187eb6bdbdb617888"
  , gossip_recieve = "21819ab0c9374188b0dbb9621e1696cd";
  
var outgoing
  , incoming;


function Gossip(){
  
  
};

Gossip.prototype.write = function(data){};

Gossip.prototype.read = function(){};

noble.startScanning([service_uuid]);

noble.on('discover', function(peripheral){
  console.log('Peripheral found:', peripheral.advertisement);
  if (peripheral.advertisement.serviceUuids.indexOf(service_uuid) > -1){
    peripheral.connect();
    
    peripheral.on('connect', onConnect);
  }
});

function onConnect(){
  //this == peripheral
  console.log('Reach connected');
  
  noble.stopScanning();
  this.on('disconnect', function(){
    noble.startScanning([service_uuid]);
  });
  
  this.discoverSomeServicesAndCharacteristics([service_uuid], [gossip_send, gossip_recieve], function(err, services, characteristics){
    for (var i=0; i<services.length; i++){
      if (services[i].uuid == service_uuid){
        for (var i=0; i<characteristics.length; i++){
          if (characteristics[i].uuid == gossip_send){
            console.log('Outgoing identified');
            outgoing = characteristics[i];
            outgoing.on('write', onWrite);
          } else if (characteristics[i].uuid == gossip_recieve){
            console.log('Incoming identified');
            incoming = characteristics[i];
            incoming.notify(true, function(err){
              err && console.log('Could not subscribe to notifications:', err);
            });
            incoming.on('read', onRead);
          };
        }
      }
    }
  });
};

function onServicesDiscover(services){
  //this == peripheral
  console.log('Service discovered:', services);
  for (var i=0; i<services.length; i++){
    if (services[i].uuid == service_uuid){
      services[i].discoverCharacteristics([gossip_send, gossip_recieve]);
      services[i].on('characteristicsDiscover', onCharacteristicDiscover);
    }
  }
};

function onCharacteristicDiscover(characteristics){
  //this == service
  console.log('Characteristics discovered:', characteristics);
  for (var i=0; i<characteristics.length; i++){
    if (characteristics[i].uuid == gossip_send){
      console.log('Outgoing identified');
      outgoing = characteristics[i];
      outgoing.on('write', onWrite);
    } else if (characteristics[i].uuid == gossip_recieve){
      console.log('Incoming identified');
      incoming = characteristics[i];
      incoming.notify(true, function(err){
        err && console.log('Could not subscribe to notifications:', err);
      });
      incoming.on('read', onRead);
    };
  }
};

function onRead(data, isNotification){
  //this == characteristic
  console.log('Read:', data);
  if (isNotification){
    outgoing.write(new Buffer([0x80, 0x20, 0x20]), false);
  }
};

function onWrite(){
  //this == characteristic
};
