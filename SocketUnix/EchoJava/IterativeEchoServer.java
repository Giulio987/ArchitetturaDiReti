import java.io.*;
import java.net.*;


public class IterativeEchoServer
{
    static final int PORT = 7;

    public static void main(String args[])
    {
        try {
            ServerSocket ss = new ServerSocket(PORT);

            while(true) {
                Socket ns = ss.accept();
                BufferedReader networkIn = new BufferedReader(new InputStreamReader(ns.getInputStream(),"UTF-8"));
                BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(ns.getOutputStream(),"UTF-8"));

                String line;
                while ((line = networkIn.readLine()) != null) {
                    System.out.println("Ricevuto: " + line);
                    networkOut.write(line);
                    networkOut.newLine();
                    networkOut.flush();
                }

                ns.close();
            }
        }
        catch(IOException e) {
            System.err.println(e);
            System.exit(1);
        }
    }
}



