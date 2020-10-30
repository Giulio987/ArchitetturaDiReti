import java.io.*;
import java.net.*;
import java.util.Date;


public class IterativeDaytimeServer
{
    static final int PORT = 13;

    public static void main(String args[])
    {
        try {
            ServerSocket ss = new ServerSocket(PORT);

            while(true) {
                Socket ns = ss.accept();

                BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(ns.getOutputStream(),"UTF-8"));

                Date now = new Date();
                networkOut.write(now.toString());
                networkOut.newLine();
                networkOut.flush();

                ns.close();
            }
        }
        catch(IOException e) {
            System.err.println(e);
            System.exit(1);
        }
    }
}

