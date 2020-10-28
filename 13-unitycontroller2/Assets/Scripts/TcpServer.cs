using MQTTnet;
using MQTTnet.Client.Receiving;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;


public class TcpServer : MonoBehaviour
{

    public string ipAddress = "0.0.0.0";
    public int port = 11111;

    private TcpListener listener;
    private TcpClient client;
    private NetworkStream stream;
    private BinaryReader reader;
    private BinaryWriter writer;
    private IMqttApplicationMessageReceivedHandler handler;


    public void Start()
    {
        var ip = IPAddress.Parse(ipAddress);
        listener = new TcpListener(ip, port);
        listener.Start();
        listener.BeginAcceptSocket(DoAcceptTcpClientCallback, null);
    }


    private void DoAcceptTcpClientCallback(IAsyncResult ar)
    {
        client = listener.EndAcceptTcpClient(ar);
        print("Connect: " + client.Client.RemoteEndPoint);

        stream = client.GetStream();
        reader = new BinaryReader(stream);
        writer = new BinaryWriter(stream);

        // 接続が切れるまで送受信を繰り返す
        while (client.Connected)
        {
            var len = reader.ReadByte();
            var topicBytes = reader.ReadBytes(len);
            var topic = Encoding.UTF8.GetString(topicBytes);
            len = reader.ReadByte();
            byte[] payload = { };
            if (len > 0)
            {
                payload = reader.ReadBytes(len);
            }
            Debug.Log($"topic={topic}, payload={payload.Length}bytes");
            if (handler != null)
            {
                var message = new MqttApplicationMessage
                {
                    Topic = topic,
                    Payload = payload,
                };
                handler.HandleApplicationMessageReceivedAsync(new MqttApplicationMessageReceivedEventArgs("hoge", message));
            }

            // クライアントの接続が切れたら
            if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0))
            {
                print("Disconnect: " + client.Client.RemoteEndPoint);
                break;
            }
        }

        Cleanup();
    }


    private void Cleanup()
    {
        writer.Dispose();
        writer = null;
        reader.Dispose();
        reader = null;
        client.Close();
        client = null;
    }


    protected virtual void OnApplicationQuit()
    {
        if (listener != null) listener.Stop();
        if (client != null) client.Close();
    }


    public void UseApplicationMessageReceivedHandler(IMqttApplicationMessageReceivedHandler handler)
    {
        this.handler = handler;
    }


    internal void Publish(MqttApplicationMessage message)
    {
        if (writer == null)
        {
            return;
        }

        using (var s = new MemoryStream())
        using (var w = new BinaryWriter(s))
        {
            // w.Write((byte)message.Topic.Length);
            // w.Write(Encoding.UTF8.GetBytes(message.Topic));
            w.Write(message.Topic);
            w.Write((byte)message.Payload.Length);
            w.Write(message.Payload);

            var bytes = s.ToArray();
            stream.Write(bytes, 0, bytes.Length);
            stream.Flush();
        }

        // writer.Write((byte)message.Topic.Length);
        // writer.Write(Encoding.UTF8.GetBytes(message.Topic));
        // writer.Write((byte)message.Payload.Length);
        // writer.Write(message.Payload);
    }

}
