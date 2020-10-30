import java.io.*;
import java.net.*;

public class ServerRcpConc implements Runnable {
    Socket s;

    public static void main(String args[]) {
        Thread t;
        ServerSocket ss;
        Socket s;

        if (args.length != 1) {
            System.out.println("Uso: java ServerRcpConc porta");
            System.exit(1);
        }

        try {
            ss = new ServerSocket(Integer.parseInt(args[0]));
            System.out.println("Attesa connessione su porta " + ss.getLocalPort());
            while (true) {
                s = ss.accept();
                t = new Thread(new ServerRcpConc(s));
                t.start();
            }
        }
        catch (IOException e) { 
            System.err.println(e); 
        }
    }

    public ServerRcpConc(Socket _s) {
        this.s = _s;
    }

    public void run() {

        final int BUFDIM = 1024;

        try {
            System.out.println("Thread numero " + Thread.currentThread());
            System.out.println("Connessione creata con " + s);

            BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream(),"UTF-8"));
            OutputStream out = s.getOutputStream();

            String request = in.readLine();
            System.out.println("Richiesto file: " + request);
            File file = new File(request);
            if (file.exists()) {
                byte yes[] = new String("S").getBytes("UTF-8");
                out.write(yes, 0, yes.length);
                out.flush();

                byte buffer[] = new byte[BUFDIM];
                int bytesRead;
                FileInputStream fileIn = new FileInputStream(file);
                while ((bytesRead = fileIn.read(buffer, 0, BUFDIM)) != -1) {
                    out.write(buffer, 0, bytesRead);
                }
                fileIn.close();
                out.flush();
            } else {
                byte no[] = new String("N").getBytes("UTF-8");
                out.write(no, 0, no.length);
                out.flush();
            }
            s.close();

            System.out.println("Fine servizio thread numero " + Thread.currentThread());
        }
        catch (IOException e) {
            System.err.println(e);
            System.exit(2);
        }
    }
}

