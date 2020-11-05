import java.io.*;
import java.net.*;


public class RcpClient {
    static final int BUFDIM = 4096;

    public static void main(String[] args) {

        if (args.length != 4) {
            System.err.println("Uso: java RcpClient nodoserver portaserver nomefilesorgente nomefiledest");
            System.exit(1);
        }

        File theFile = new File(args[3]);
        if (theFile.exists()) {
            System.err.println("Il file esiste già. Termino.");
            System.exit(2);
        }

        try {
            FileInputStream fileIn;
            Socket s = new Socket(args[0], Integer.parseInt(args[1]));

            BufferedWriter out = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(),"UTF-8"));
            out.write(args[2]);
            out.newLine();
            out.flush();

            InputStream in = s.getInputStream();
            byte buf[] = new byte[1];
            in.read(buf, 0, 1);
            String risposta = new String(buf, "UTF-8");

            System.out.println(risposta);

            if (risposta.equals("S")) {
                byte buffer[] = new byte[BUFDIM];
                int bytesRead;
                FileOutputStream fileOut = new FileOutputStream(theFile);
                while ((bytesRead = in.read(buffer, 0, BUFDIM)) != -1) {
                    fileOut.write(buffer, 0, bytesRead);
                }
                fileOut.close();
            }

            s.close();
        }
        catch (IOException e) { 
            System.err.println("Errore:" + e);
            System.exit(3);
        }
    }
}



