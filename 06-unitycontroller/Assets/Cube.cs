public class Cube
{

    public string Address { get; }
    public bool IsConnected { get; set; }
    public Client Bridge { get; set; }


    public Cube(string address)
    {
        Address = address;
        IsConnected = false;
    }


    public void SetLamp()
    {
        byte[] data = { 0x03, 0x00, 0x01, 0x01, 0xff, 0xff, 0xff };
        Bridge.WriteLampCommand(this, data);
    }


    public override string ToString() => $"[Cube Address={Address} IsConnected={IsConnected}]";

}
