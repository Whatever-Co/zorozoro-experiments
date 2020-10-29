using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using System.Text;
using UnityEngine;


public class ScannerReceiver : MonoBehaviour
{

    public string ipAddress = "0.0.0.0";
    public int port = 11122;

    public event Action<string> OnNewCube;

    private TcpListener listener;


    public void Start()
    {
        var ip = IPAddress.Parse(ipAddress);
        listener = new TcpListener(ip, port);
        listener.Start();
        Task.Run(() =>
        {
            while (listener != null)
            {
                var client = listener.AcceptTcpClient();
                print("Connect: " + client.Client.RemoteEndPoint);
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
