using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using UnityEngine;


public class ScannerReceiver : MonoBehaviour
{

    private readonly IPAddress SERVER_ADDRESS = IPAddress.Any;
    private readonly int SERVER_PORT = 11122;

    public event Action<string> OnNewCube;

    private TcpListener listener;


    public void Start()
    {
        listener = new TcpListener(SERVER_ADDRESS, SERVER_PORT);
        listener.Start();
        Task.Run(() =>
        {
            while (listener != null)
            {
                var client = listener.AcceptTcpClient();
                print("Scanner connection accepted: " + client.Client.RemoteEndPoint);
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
                Debug.Log($"command={command}, address={address}");
                OnNewCube.Invoke(address);
            }

            if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0))
            {
                Debug.LogWarning("Disconnect: " + client.Client.RemoteEndPoint);
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
    }

}
