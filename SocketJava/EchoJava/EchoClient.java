import java.io.*;
import java.net.*;


public class EchoClient
{
    public static void main(String args[])
    {
        try {
            if (args.length != 1) {
                System.err.println("Errore! La sintassi corretta è: java EchoClient hostname");
                System.exit(1);
            }
            String hostname = args[0];
            Socket theSocket = new Socket(hostname, 7);
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(),"UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(theSocket.getOutputStream(),"UTF-8"));

            System.out.println("Connected to echo server");
            while (true) {
                String theLine = userIn.readLine();
                if (theLine.equals(".")) break;
                networkOut.write(theLine);
                networkOut.newLine();
                networkOut.flush();
                System.out.println(networkIn.readLine());
            }
        }
        catch (IOException e) {
            System.err.println("Errore: " + e);
            System.exit(2);
        }
    }
}
