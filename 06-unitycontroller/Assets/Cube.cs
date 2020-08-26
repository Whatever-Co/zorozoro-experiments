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


    public override string ToString() => $"[Cube Address={Address} IsConnected={IsConnected}]";

}
