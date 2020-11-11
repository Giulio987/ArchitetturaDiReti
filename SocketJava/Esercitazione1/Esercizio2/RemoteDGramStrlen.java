import java.net.*;
import java.io.*;


public class RemoteDGramStrlen {
    public static void main(String args[]){
        if(args.length != 2){
            System.err.println("Inserire host e porta");
            System.exit(1);
        }
        try{
            DatagramSocket ds = new DatagramSocket();
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));
            System.out.println("Inserire la stringa");
            String line;
            byte[] msg;
            while(!(line = tastiera.readLine()).equals("fine")){

                System.out.println("Inserire la stringa");
                msg = line.getBytes("UTF-8");

                DatagramPacket dpinvio = new DatagramPacket(msg, msg.length, InetAddress.getByName(args[0]),Integer.parseInt(args[1]));
                ds.send(dpinvio);

                byte response[] = new byte[2048];
                DatagramPacket dpresp = new DatagramPacket(response, response.length);
                ds.receive(dpresp);
                String risposta = new String(dpresp.getData(), 0, dpresp.getLength(), "UTF-8");
                
                System.out.println("La frase è lunga " + risposta + " caratteri");
                System.out.println("Inserire la stringa");
            }
            ds.close();
        }catch(IOException e){
            e.getMessage();
        }
    }
}
