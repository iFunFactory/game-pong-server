using System;
  using funapi.service.multicast_message;
  using funapi.management.maintenance_message;
  using funapi.network.ping_message;

  using pong_messages;
  

  namespace Fun {
    public enum MessageType {
      multicast = 8,  // FunMulticastMessage
      cs_ping = 9,  // FunPingMessage
      pbuf_maintenance = 15,  // MaintenanceMessage
      pbuf_echo = 16,  // PbufEchoMessage
      pbuf_another = 17,  // PbufAnotherMessage
    }

    public enum MulticastMessageType {
      chat = 8,  // FunChatMessage
      pbuf_hello = 9,  // PbufHelloMessage
    }

    public class MessageTable {
      public static string Lookup(MessageType msg_type) {
        switch(msg_type) {
          case MessageType.multicast: return "multicast";
          case MessageType.cs_ping: return "cs_ping";
          case MessageType.pbuf_maintenance: return "pbuf_maintenance";
          case MessageType.pbuf_echo: return "pbuf_echo";
          case MessageType.pbuf_another: return "pbuf_another";
          default: return "";
        }
      }

      public static Type GetType(MessageType msg_type) {
        switch(msg_type) {
          case MessageType.multicast: return typeof(FunMulticastMessage);
          case MessageType.cs_ping: return typeof(FunPingMessage);
          case MessageType.pbuf_maintenance: return typeof(MaintenanceMessage);
          case MessageType.pbuf_echo: return typeof(PbufEchoMessage);
          case MessageType.pbuf_another: return typeof(PbufAnotherMessage);
          default: return null;
        }
      }

      public static string Lookup(MulticastMessageType msg_type) {
        switch(msg_type) {
          case MulticastMessageType.chat: return "chat";
          case MulticastMessageType.pbuf_hello: return "pbuf_hello";
          default: return "";
        }
      }
    }
  }
  