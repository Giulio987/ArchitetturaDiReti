import java.io.*;
import java.net.*;

public class EchoReverseClientRetry
{
    private static final int TIMEOUT = 5000; // Receive timeout (in milliseconds)
    private static final int MAX_TRIES = 5;  // Numero massimo di ritrasmissioni

    public static void main(String[] args)
    {
        // Controllo argomenti
        if (args.length != 2) {
            System.err.println("Uso: java EchoReverseClientRetry nodoserver portaserver");
            System.exit(1);
        }

        try {
            // Leggo una stringa da tastiera
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));
            System.out.println("Inserisci la stringa da invertire: ");
            String msg = tastiera.readLine();

            // Levo spazi bianchi all'inizio e alla fine della stringa
            msg = msg.trim();

            // Creo la socket
            DatagramSocket sock = new DatagramSocket();

            // Setto timeout I/O per la socket
            sock.setSoTimeout(TIMEOUT);

            // Ottengo indirizzo IP e numero porta del server
            InetAddress remoteAddr = InetAddress.getByName(args[0]);
            int remotePort = Integer.parseInt(args[1]);

            // Preparo datagramma di richiesta
            byte[] buf = msg.getBytes("UTF-8");
            DatagramPacket packet = new DatagramPacket(buf, buf.length, remoteAddr, remotePort);
            DatagramPacket recvPacket;

            // Uso un ciclo per provare a risottomettere richieste di cui non
            // ho ricevuto risposta (semantica "at-least-once")
            int numTries = 0;
            boolean receivedResponse = false;
            do {
                // Invio la richiesta
                sock.send(packet);

                // Preparo datagramma vuoto per leggere la risposta
                byte[] recvBuf = new byte[512];
                recvPacket = new DatagramPacket(recvBuf, recvBuf.length);

                try {
                    // Ricezione datagramma di risposta
                    sock.receive(recvPacket);

                    // Ho ricevuto la risposta
                    receivedResponse = true;
                }
                catch (SocketTimeoutException e) {
                    // Timeout scaduto senza ricevere la risposta
                    numTries += 1;
                }
            } while ((!receivedResponse) && (numTries < MAX_TRIES));

            if (receivedResponse) {
                // Stampo contenuto della risposta a video
                System.out.println(new String(recvPacket.getData(), "UTF-8"));
            } else {
                System.out.println("Nessuna risposta dopo " + MAX_TRIES + " tentativi. Esco.");
            }
        }
        catch(IOException e){
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
