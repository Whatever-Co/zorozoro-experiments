using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Linq;
using UnityEngine;


public class Client
{

    public static readonly int MAX_CUBES_PER_CLIENT = 8;


    public enum Mode
    {
        NotInitialized,
        Scanner,
        Bridge,
    }


    private TcpClient client;
    private bool isRunning = false;

    public Mode mode { get; private set; }
    public IPEndPoint EndPoint { get => (IPEndPoint)client?.Client?.RemoteEndPoint; }
    public string Address { get => EndPoint?.Address.ToString(); }

    public event Action<Client, Mode> OnHello;
    public event Action<Client, Cube> OnCubeFound;

    private NetworkStream stream;
    private StreamReader reader;
    private StreamWriter writer;

    // for bridges
    private Cube connectingCube;
    private List<Cube> cubes = new List<Cube>();
    public int NumConnectedCubes { get => cubes?.Count ?? 0; }
    public bool IsBusy { get; private set; } = false;


    public Client(TcpClient client)
    {
        this.client = client;
    }


    public void Start()
    {
        isRunning = true;
        try
        {
            using (stream = client.GetStream())
            using (reader = new StreamReader(stream))
            using (writer = new StreamWriter(stream))
            {
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
                                HandleHello(tokens[1]);
                                break;
                            case "advertised":
                                HandleAdvertise(tokens[1], tokens[3]);
                                break;
                            case "connected":
                                HandleConnected(tokens[1]);
                                break;
                            case "disconnected":
                                HandleDisconnected(tokens[1]);
                                break;
                            case "battery":
                                HandleBattery(tokens[1], tokens[2]);
                                break;
                        }
                    }
                    if (client.Client.Poll(1000, SelectMode.SelectRead) && (client.Client.Available == 0))
                    {
                        break;
                    }
                }
            }
        }
        catch (SocketException e)
        {
            Debug.Log(e);
        }
        finally
        {
            Stop();
        }
    }


    private void HandleHello(string modeStr)
    {
        switch (modeStr)
        {
            case "scanner":
                mode = Mode.Scanner;
                break;
            case "bridge":
                mode = Mode.Bridge;
                break;
            default:
                Debug.LogWarning("unknown mode " + modeStr);
                break;
        }
        if (mode != Mode.NotInitialized)
        {
            OnHello?.Invoke(this, mode);
        }

    }


    private void HandleAdvertise(string address, string name)
    {
        if (name == "toio Core Cube")
        {
            Debug.Log("Found toio Cube!!!");
            OnCubeFound?.Invoke(this, new Cube(address));
        }
    }


    private void HandleConnected(string address)
    {
        IsBusy = false;
        if (connectingCube.Address == address)
        {
            connectingCube.Bridge = this;
            cubes.Add(connectingCube);
            Debug.Log($"new cube {address} added");
            connectingCube.SetLamp();
        }
        else
        {
            Debug.LogWarning("???");
        }
        connectingCube = null;
    }


    private void HandleDisconnected(string address)
    {
        IsBusy = false;
        if (connectingCube != null)
        {
            Debug.Log($"connect failed {connectingCube.Address}");
            connectingCube = null;
        }
        else
        {
            var remove = cubes.Where(c => c.Address == address && c.Bridge == this).ToList();
            foreach (var c in remove)
            {
                cubes.Remove(c);
                Debug.Log("cube removed " + c.Address);
            }
        }
    }


    private void HandleBattery(string address, string battery)
    {
        if (int.TryParse(battery, out var value))
        {
            Debug.Log($"");
        }
    }


    public void Connect(Cube cube)
    {
        Debug.Log($"connecting {cube.Address} through {Address}");
        IsBusy = true;
        connectingCube = cube;
        writer.WriteLine($"connect\t{cube.Address}");
    }


    public void Stop()
    {
        isRunning = false;
        if (client != null)
        {
            client.Client.Close();
            client.Close();
            client.Dispose();
            client = null;
        }
        stream = null;
        reader = null;
        writer = null;
    }


    public void WriteLampCommand(Cube cube, byte[] data)
    {
        if (writer == null)
        {
            Debug.LogWarning("writer is null");
            return;
        }
        if (data.Length > 255)
        {
            Debug.LogWarning("command is too long");
        }
        writer.WriteLine($"lamp\t{cube.Address}");
        stream.WriteByte((byte)data.Length);
        stream.Write(data, 0, data.Length);
        stream.Flush();
        writer.WriteLine("ping");
    }


    public override string ToString() => $"[Client Mode={mode} Address={Address}]";

}
