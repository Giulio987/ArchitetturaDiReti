import java.io.*;
import java.net.*;

public class EchoReverseClient
{
    public static void main(String[] args)
    {
        // Controllo argomenti
        if (args.length != 2){
            System.out.println("Uso: java EchoReverseClient nodoserver portaserver");
            System.exit(1);
        }

        try{
            // Leggo stringa da tastiera
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));
            System.out.println("Inserisci la stringa da invertire: ");
            String msg = tastiera.readLine();

            // Levo spazi bianchi all'inizio e alla fine della stringa
            msg = msg.trim();

            // Creo la socket
            DatagramSocket sock = new DatagramSocket();

            System.out.println("Mando richiesta: " + msg);

            // Preparo il datagramma di richiesta
            byte[] buf = msg.getBytes("UTF-8");
            InetAddress remoteAddr = InetAddress.getByName(args[0]);
            int remotePort = Integer.parseInt(args[1]);
            DatagramPacket packet = new DatagramPacket(buf, buf.length, remoteAddr, remotePort);

            // Invio datagramma al server
            sock.send(packet);

            // Preparo datagramma di risposta
            buf = new byte[1024];
            packet = new DatagramPacket(buf, buf.length);

            // Ricezione datagramma di risposta
            sock.receive(packet);

            // Stampo contenuto della risposta a video
            System.out.println("Ricevo risposta: " + new String(packet.getData(),"UTF-8"));

            // Chiudo la socket
            sock.close();
        }
        catch(IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
