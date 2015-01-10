var tessel = require('tessel')
  , bleLib = require('ble-ble113a')
  , bleadvertise = require('bleadvertise');
  
var protocol = require('./protocol.js');
var command = protocol.Command
  , NO_CHANGE = protocol.NO_CHANGE
  , CommState = protocol.CommState;

var aux = tessel.port['B'];
var gossip = new Gossip();

var ble = bleLib.use(tessel.port['A'], function(){
  console.log('BLE ready');
  var advertisement = bleadvertise.serialize({
    flags: [0x02, 0x04],
    shortName: 'Reach',
    incompleteUUID128: ['d752c5fb13804cd5b0efcac7d72cff20']
  });
  ble.setAdvertisingData(advertisement, function(err){
    if (!err){
      console.log('Advertising data set');
      ble.startAdvertising(function(err){
        if(!err){
          console.log('Advertising started');
        }
      });
    }
  });
});

ble.on('connect', function(connection){
  console.log('Connected');
});

ble.on('remoteWrite', function(connection, index, value){
  console.log('write:', index, value);
  if (index === 0){
    gossip.handleIncoming(value);
  }
});

ble.on('remoteReadRequest', function(connection, index){
  console.log('read request:', index);
});

ble.on('remoteNotification', function(connection, index){
  if (index === 1){
    console.log('Notifications ready');
    ble.writeLocalValue(1, new Buffer([0x01]));
  }
});

ble.on('disconnect', function(connection, cause){
  console.log('Disconnected');
  ble.startAdvertising();
});


function Gossip(){
  this.state = CommState.Idle;
  this.spi;
}

Gossip.prototype.handleIncoming = function(data){
  var base = data[0];
  var cmdBase = data[1];
  if (base === 0x80 && cmdBase === 0x20){
    this.handleSPI(data.slice(2));
  }
}

Gossip.prototype.handleSPI = function(incoming){
  command = incoming[0];
  console.log("SPI Command:", command);
  switch (command){
    case command.SPISETCLOCKDIVISOR: {
      var payload = incoming.slice(1);
      var param = payload[0];
      self.spi.setClockSpeed(param);
      outgoing[0] = command.SPISETCLOCKDIVISOR;
      return 1;
    }
    case command.SPISETMODE: {
      var payload = incoming.slice_from(1);
      var param = payload[0];
      self.spi.setDataMode(param);
      outgoing[0] = command.SPISETMODE;
      return 1;
    }
    case command.SPISETROLE: {
      var payload = incoming.slice(1);
      var param = payload[0];
      self.spi.setRole(param);
      outgoing[0] = command.SPISETROLE;
      return 1;
    }
    case command.SPISETFRAME: {
      var payload = incoming.slice(1);
      var param = payload[0];
      self.spi.setFrameMode(param);
      outgoing[0] = command.SPISETFRAME;
      return 1;
    }
    default:
      switch (state) {
        case CommState.Idle:
          switch (command) {
            case command.SPIENABLE: {
                self.spi = aux.SPI({clockSpeed : 1000, mode:2, chipSelect:this.chipSelect, chipSelectDelayUs:500});
                self.state = CommState.SPIEnable;
                outgoing[0] = command.SPIENABLE;
                return 1;
              }
            case command.SPIDISABLE: {
              outgoing[0] = command.SPIDISABLE;
              return 1;
            }
            default:
              return 0;
          }
        case CommState.SPIEnable:
          switch (command){
            case command.SPIENABLE: {
              outgoing[0] = command.SPIENABLE;
              return 1;
            }
            case command.SPITRANSFER: {
              var length = incoming[1];
              var payload = incoming.slice(2);
              outgoing[0] = command.SPITRANSFER;
              outgoing[1] = length;
              return self.spi.transfer(payload) + 2
            }
            case command.SPIDISABLE: {
              self.spi.close();
              self.state = CommState.Idle;
              outgoing[0] = command.SPIDISABLE;
              return 1;
            }
            default:
              return 0;
          }
        default: 
          return 0;
        }
    }
}
