import java.io.*;
import java.net.*;


public class DaytimeClient
{
    public static void main(String args[])
    {
        try {
            if (args.length != 1) {
                System.out.println("Errore! La sintassi corretta è: java DaytimeClient hostname");
                System.exit(1);
            }
            String hostname = args[0];
            Socket theSocket = new Socket(hostname, 13);
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(),"UTF-8"));
            System.out.println("Connected to daytime server");
            String theLine;
            while ((theLine = networkIn.readLine()) != null) {
                System.out.println(theLine);
            }
        }
        catch (IOException e) {
            System.err.println(e);
            System.exit(2);
        }
    }
}
