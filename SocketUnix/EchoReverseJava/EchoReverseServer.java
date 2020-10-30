import java.io.*;
import java.net.*;
import java.util.Arrays;


public class EchoReverseServer
{
    public static void main(String[] args)
    {
        // Controllo argomenti
        if (args.length != 1) {
            System.err.println("Uso: java EchoReverseServer porta");
            System.exit(1);
        }

        try {
            DatagramSocket sock = new DatagramSocket(Integer.parseInt(args[0]));
            System.out.println("Creata DatagramSocket locale sulla porta " + args[0]);

            while(true) {
                // Preparo datagramma per ricezione richiesta
                byte[] buf = new byte[2048];
                DatagramPacket packet = new DatagramPacket(buf, buf.length);

                // Ricevo un pacchetto
                sock.receive(packet);

                // Ne estraggo il contenuto
                byte[] requestBuf = Arrays.copyOf(packet.getData(), packet.getLength());

                // Lo converto a stringa considerando encoding UTF-8
                String request = new String(requestBuf, "UTF-8");

                // Log della richiesta a video
                System.out.println("requestBuf: " + Arrays.toString(requestBuf));
                System.out.println("Ricevuta richiesta da " +
                                   packet.getAddress().getHostAddress() + ":" +
                                   packet.getPort() + " ('" + request + "')");

                // Inversione stringa
                // https://stackoverflow.com/questions/7569335/reverse-a-string-in-java
                String response = new StringBuilder(request).reverse().toString();

                // Conversione ad array di byte in encoding UTF-8
                byte[] respBuf = response.getBytes("UTF-8");
                
                // Log della risposta a video
                System.out.println("Invio risposta a " +
                                   packet.getAddress().getHostAddress() + ":" +
                                   packet.getPort() + " ('" + response + "')");
                System.out.println("respBuf: " + Arrays.toString(respBuf));

                // Estraggo dal pacchetto di richiesta indirizzo e porta del nodo
                InetAddress incomingAddr = packet.getAddress();
                int incomingPort = packet.getPort();

                // Creo la risposta e la invio
                packet = new DatagramPacket(respBuf, respBuf.length, 
                                            incomingAddr, incomingPort);
                sock.send(packet);

                // Log invio risposta
                System.out.println("Inviata risposta a " +
                                   incomingAddr.getHostAddress() + ":" + incomingPort);
            }
        }
        catch(IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
