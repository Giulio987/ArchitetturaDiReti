import java.net.*;
import java.io.*;

public class RemoteSquareClient {
    public static void main(String arg[]){
        if (arg.length != 2){
            System.err.println("Uso corretto: java Remo** nodoServer portaServer");
            System.exit(1);
        }
        try{
            Socket s = new Socket(arg[0], Integer.parseInt(arg[1]));
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader inSock = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
            BufferedWriter outSock = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8"));
            String numero;
            while(true){
                System.out.println("Inserire un numero di cui calcolare il quadrato. \"fine\" per uscire");
                numero = tastiera.readLine();
                if(!numero.equals("fine")){
                    try{
                        Integer.parseInt(numero);
                    }catch(NumberFormatException e){
                        System.out.println("Inserire un numero valido");
                        continue;
                    }
                    outSock.write(numero);
                    outSock.newLine();
                    outSock.flush();

                    System.out.println("Il quadrato del numero è " + inSock.readLine());
                }
                else{
                    s.close();
                    break;
                }
            } 
        }catch(IOException es){
            es.getMessage();
            System.exit(1);
        }
    }
}
