import java.io.*;
import java.net.*;


public class IterativeRcpServer {

    public static void main(String args[]) {
        final int BUFDIM = 1024;

        if (args.length != 1)  {
            System.err.println("Uso: java IterativeRcpServer porta");
            System.exit(1);
        }

        try {
            byte yes[] = new String("S").getBytes("UTF-8");
            byte no[] = new String("N").getBytes("UTF-8");
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));

            System.out.println("Attesa connessione su porta " + ss.getLocalPort());

            while (true) {
                Socket s = ss.accept();
                BufferedReader in = new BufferedReader(new InputStreamReader(s.getInputStream(),"UTF-8"));
                OutputStream out = s.getOutputStream();

                String request = in.readLine();
                System.out.println("Richiesto file: " + request);
                File file = new File(request);
                if (file.exists()) {
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
                    out.write(no, 0, no.length);
                    out.flush();
                }
                s.close();
            }
        }
        catch (IOException e) {
            System.err.println(e);
            System.exit(2);
        }
    }
}

