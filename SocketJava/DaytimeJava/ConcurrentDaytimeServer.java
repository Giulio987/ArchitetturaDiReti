import java.io.*;
import java.net.*;
import java.util.Date;


public class ConcurrentDaytimeServer
{
    static final int PORT = 13;

    public static void main(String args[])
    {
        try {
            ServerSocket ss = new ServerSocket(PORT);

            while(true) {
                Socket ns = ss.accept();
                WorkerThread t = new WorkerThread(ns);
                t.start();
            }
        }
        catch(IOException e) {
            System.err.println("Error: " + e);
            System.exit(1);
        }
    }
}



class WorkerThread extends Thread
{
    WorkerThread(Socket theSocket)
    {
        this.s = theSocket;
    }

    public void run()
    {
        try {
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(),"UTF-8"));

            Date now = new Date();
            networkOut.write(now.toString());
            networkOut.newLine();
            networkOut.flush();

            s.close();
        }
        catch(IOException e) {
            System.err.println("Error: " + e);
            System.exit(1);
        }
    }

    private Socket s;
}



