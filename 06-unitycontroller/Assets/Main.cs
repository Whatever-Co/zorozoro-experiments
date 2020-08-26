using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

// advertised   cc:06:5c:32:a1:34   toio Core Cube
// advertised   e6:b0:bf:76:91:ce   toio Core Cube

public class Main : MonoBehaviour
{

    TcpListener listener;
    bool isRunning = true;


    void Start()
    {
        var address = IPAddress.Parse("0.0.0.0");
        listener = new TcpListener(address, 12322);
        listener.Start();
        listener.BeginAcceptTcpClient(AcceptTcpClient, null);
    }


    void AcceptTcpClient(IAsyncResult result)
    {
        var client = listener.EndAcceptTcpClient(result);
        Debug.Log($"Accept tcp client: {client.Client.RemoteEndPoint}");

        listener.BeginAcceptTcpClient(AcceptTcpClient, null);

        try
        {
            var stream = client.GetStream();
            var reader = new StreamReader(stream);
            var writer = new StreamWriter(stream);
            writer.AutoFlush = true;
            while (client.Connected && isRunning)
            {
                while (!reader.EndOfStream)
                {
                    var line = reader.ReadLine();
                    Debug.Log(line);
                    var tokens = line.Split('\t');
                    var command = tokens[0];
                    switch (command)
                    {
                        case "hello":
                            switch (tokens[1])
                            {
                                case "bridge":
                                    writer.WriteLine("hello\tcontroller");
                                    writer.WriteLine("connect\te6:b0:bf:76:91:ce");
                                    break;
                            }
                            break;
                        case "advertised":
                            if (tokens[2] == "toio Core Cube")
                            {
                                Debug.Log("Found toio Cube!!!");
                            }
                            break;
                    }
                }
                if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0))
                {
                    break;
                }
            }
        }
        catch (Exception e)
        {
            Debug.LogException(e);
        }
        Debug.Log("Disconnect: " + client.Client.RemoteEndPoint);
        client.Close();
    }


    void OnApplicationQuit()
    {
        listener.Stop();
        isRunning = false;
    }


}
