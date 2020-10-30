using Microsoft.Extensions.Logging;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using UnityEngine;
using ZLogger;


public class ScannerReceiver : MonoBehaviour
{

    private static readonly ILogger<ScannerReceiver> logger = LogManager.GetLogger<ScannerReceiver>();


    private readonly IPAddress SERVER_ADDRESS = IPAddress.Any;
    private readonly int SERVER_PORT = 11122;

    public event Action<string> OnNewCube;

    private TcpListener listener;
    private TcpClient client;


    public void Start()
    {
        listener = new TcpListener(SERVER_ADDRESS, SERVER_PORT);
        listener.Start();
        Task.Run(() =>
        {
            while (listener != null)
            {
                client = listener.AcceptTcpClient();
                logger.ZLogDebug("Scanner connection accepted: {0}", client.Client.RemoteEndPoint);
                Task.Run(() => DoAcceptTcpClientCallback(client));
            }
        });
    }


    private void DoAcceptTcpClientCallback(TcpClient client)
    {
        var stream = client.GetStream();
        var reader = new BinaryReader(stream);

        while (client.Connected)
        {
            if (client.Available > 0)
            {
                var command = reader.ReadString();
                var address = reader.ReadString();
                logger.ZLogDebug($"command={command}, address={address}");
                OnNewCube.Invoke(address);
            }

            if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0))
            {
                logger.ZLogWarning("Disconnect: " + client.Client.RemoteEndPoint);
                break;
            }
        }

        reader.Dispose();
        reader = null;
        client.Close();
        client = null;
    }


    private void OnApplicationQuit()
    {
        if (listener != null)
        {
            listener.Stop();
            listener = null;
        };
        if (client != null)
        {
            client.Close();
            client = null;
        }
    }

}
