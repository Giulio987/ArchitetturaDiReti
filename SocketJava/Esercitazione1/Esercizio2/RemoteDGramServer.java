import java.io.*;
import java.net.*;

public class RemoteDGramServer {
    public static void main(String args[]){

        if (args.length != 1) {
            System.err.println("Usage: java RemoteDGramServer porta");
            System.exit(1);
        }
        try{
            DatagramSocket ds = new DatagramSocket(Integer.parseInt(args[0]));
            while(true){
                byte[] requestpkg = new byte[2048];
                DatagramPacket in = new DatagramPacket(requestpkg, requestpkg.length);
                ds.receive(in);
                int length = new String(in.getData(),0,in.getLength(), "UTF-8").length();
                byte[] send = ("" + length).getBytes("UTF-8");
                DatagramPacket out = new DatagramPacket(send, send.length, in.getAddress(),in.getPort());
                ds.send(out);
            }
        }catch(IOException e){
            e.getMessage();
            System.exit(1);
        }
    }
}
