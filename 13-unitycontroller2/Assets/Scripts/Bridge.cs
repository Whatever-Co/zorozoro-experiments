using Microsoft.Extensions.Logging;
using MQTTnet;
using System;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using ZLogger;


public class Bridge
{

    static readonly ILogger<Bridge> logger = LogManager.GetLogger<Bridge>();


    public string Address { get; private set; }
    public int AvailableSlot { get; set; } = 0;
    public bool ConnectingCube { get; private set; } = false;

    public event Action<Bridge, string, string, byte[]> OnMessage;
    public event Action<Bridge> OnDisconnected;

    private TcpClient client;
    private NetworkStream stream;
    private BinaryReader reader;
    private BinaryWriter writer;


    public Bridge(TcpClient client)
    {
        this.client = client;
        Address = client.Client.RemoteEndPoint.ToString().Split(':')[0];
        OnMessage += HandleMessage;
    }


    public void Start()
    {
        Task.Run(() =>
        {
            Debug.Log($"Start: {this}");

            stream = client.GetStream();
            reader = new BinaryReader(stream);
            writer = new BinaryWriter(stream);

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
                var token = topic.Split('/');
                var address = token[0];
                var command = token.Length > 1 ? token[1] : "";
                Debug.Log($"address={address}, command={command}, payload={payload.Length}bytes");
                OnMessage.Invoke(this, address, command, payload);

                if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0))
                {
                    Debug.Log("Disconnect: " + client.Client.RemoteEndPoint);
                    break;
                }
            }

            writer.Dispose();
            writer = null;
            reader.Dispose();
            reader = null;
            client.Close();
            client = null;

            OnDisconnected?.Invoke(this);

            Debug.Log($"Done: {this}");
        });
    }


    public void Stop()
    {
        if (client != null)
        {
            client.Close();
        }
    }


    public void ConnectToCube(byte[] cubeAddress)
    {
        ConnectingCube = true;
        Publish($"{Address}/newcube", cubeAddress);
    }


    private void HandleMessage(Bridge bridge, string address, string command, byte[] payload)
    {
        switch (command)
        {
            case "available":
                AvailableSlot = payload[0];
                Debug.Log(this);
                break;

            case "connected":
                ConnectingCube = false;
                break;

            case "disconnected":
                break;
        }
    }


    public void SendMotor(string address, byte[] payload)
    {
        Publish(address + "/motor", payload);
    }


    public void SendLamp(string address, byte[] payload)
    {
        Publish(address + "/lamp", payload);
    }


    public void Publish(MqttApplicationMessage message)
    {
        Publish(message.Topic, message.Payload);
    }


    public void Publish(string topic, byte[] payload)
    {
        if (writer == null)
        {
            return;
        }

        using (var s = new MemoryStream())
        using (var w = new BinaryWriter(s))
        {
            w.Write(topic);
            w.Write((byte)payload.Length);
            w.Write(payload);

            var bytes = s.ToArray();
            stream.Write(bytes, 0, bytes.Length);
            stream.Flush();
        }
    }


    public override string ToString()
    {
        return $"[Bridge Address={Address}, AvailableSlot={AvailableSlot}, ConnectingCube={ConnectingCube}]";
    }

}
