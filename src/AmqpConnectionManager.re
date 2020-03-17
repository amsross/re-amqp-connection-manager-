type url = string;
type urls = array(url);

exception ConnectionError(Js.Exn.t);

module Queue = {
  type name = string;
  type message = {content: Node.Buffer.t};
};

module Exchange = {
  type name;
};

module Channel = {
  type t;
  type name;
  type ack = Queue.message => unit;
  type nack = Queue.message => unit;

  [@bs.send] external ack: (t, Queue.message) => unit = "ack";
  [@bs.send] external nack: (t, Queue.message) => unit = "nack";

  module Config = {
    type nonrec t = {
      .
      "json": bool,
      "setup": t => Js.Promise.t(unit),
    };
  };

  [@bs.send]
  external assertExchange:
    (t, Exchange.name, string, Js.t('options)) => Js.Promise.t(Exchange.name) =
    "assertExchange";

  [@bs.send]
  external assertQueue:
    (t, Queue.name, Js.t('options)) =>
    Js.Promise.t({
      .
      "queue": string,
      "messageCount": int,
      "consumerCount": int,
    }) =
    "assertQueue";

  [@bs.send]
  external bindQueue:
    (t, Queue.name, Exchange.name, string) => Js.Promise.t(unit) =
    "bindQueue";

  [@bs.send] external prefetch: (t, int) => Js.Promise.t(unit) = "prefetch";

  [@bs.send]
  external consume: (t, Queue.name, 'a => unit) => Js.Promise.t(unit) =
    "consume";
};

module ChannelWrapper = {
  type t;
  type name;
  type routingKey = string;
  type ack = Queue.message => unit;
  type nack = Queue.message => unit;

  [@bs.send] external ack: (t, Queue.message) => unit = "ack";
  [@bs.send] external nack: (t, Queue.message) => unit = "nack";
  [@bs.send] external queueLength: t => int = "queueLength";
  [@bs.send] external close: t => unit = "close";

  [@bs.send]
  external on:
    (
      t,
      [@bs.string] [
        | `connect(unit => unit)
        | `error((Js.Exn.t, Channel.name) => unit)
        | `close(unit => unit)
      ]
    ) =>
    t =
    "on";

  [@bs.send]
  external publish':
    (t, Exchange.name, routingKey, Js.Json.t, Js.t('options)) =>
    Js.Promise.t(unit) =
    "publish";

  let publish = (t, e, k, m, o) =>
    publish'(t, e, k, m, o) |> Js.Promise.(then_(_ => resolve(m)));

  [@bs.send]
  external sendToQueue':
    (t, Queue.name, Js.Json.t, Js.t('options)) => Js.Promise.t(unit) =
    "sendToQueue";

  let sendToQueue = (t, q, m, o) =>
    sendToQueue'(t, q, m, o) |> Js.Promise.(then_(_ => resolve(m)));
};

module AmqpConnectionManager = {
  type t;
  module Options = {
    type t('connectionOptions) = Js.t('connectionOptions);
  };

  [@bs.send] external isConnected: t => bool = "isConnected";
  [@bs.send] external close: t => unit = "close";
  [@bs.send]
  external on:
    (
      t,
      [@bs.string] [
        | `connect(
            {
              .
              "connection": t,
              "url": url,
            } =>
            unit,
          )
        | `disconnect({. "err": Js.Exn.t} => unit)
      ]
    ) =>
    t =
    "on";

  [@bs.send]
  external createChannel: (t, Channel.Config.t) => ChannelWrapper.t =
    "createChannel";
};

[@bs.module "amqp-connection-manager"]
external connect:
  (urls, ~options: AmqpConnectionManager.Options.t('a)=?, unit) =>
  AmqpConnectionManager.t =
  "connect";