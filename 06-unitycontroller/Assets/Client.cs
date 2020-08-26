using System;
using System.Net;
using System.Net.Sockets;
using System.IO;
using UnityEngine;


public class Client
{

    public enum Mode
    {
        NotInitialized,
        Scanner,
        Bridge,
    }


    private TcpClient client;
    private bool isRunning = false;

    public Mode mode { get; private set; }
    public IPEndPoint EndPoint { get => (IPEndPoint)client.Client.RemoteEndPoint; }
    public string Address { get => EndPoint.Address.ToString(); }


    public event Action<Client, Mode> OnHello;
    public event Action<Client, string> OnNewCube;


    public Client(TcpClient client)
    {
        this.client = client;
    }


    public void Start()
    {
        isRunning = true;
        var stream = client.GetStream();
        var reader = new StreamReader(stream);
        var writer = new StreamWriter(stream);
        writer.AutoFlush = true;
        while (client.Connected && isRunning)
        {
            while (!reader.EndOfStream)
            {
                var line = reader.ReadLine();
                // Debug.Log(line);
                var tokens = line.Split('\t');
                var command = tokens[0];
                switch (command)
                {
                    case "hello":
                        switch (tokens[1])
                        {
                            case "scanner":
                                mode = Mode.Scanner;
                                break;
                            case "bridge":
                                mode = Mode.Bridge;
                                break;
                        }
                        if (mode != Mode.NotInitialized)
                        {
                            OnHello?.Invoke(this, mode);
                        }
                        break;
                    case "advertised":
                        if (tokens[3] == "toio Core Cube")
                        {
                            Debug.Log("Found toio Cube!!!");
                            OnNewCube?.Invoke(this, tokens[1]);
                        }
                        break;
                }
            }
            if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0))
            {
                break;
            }
        }

        Debug.Log("Disconnect: " + client.Client.RemoteEndPoint);
        client.Close();
    }


    public void Stop()
    {
        isRunning = false;
    }

}
