import java.net.*;
import java.io.*;

public class  Conto_corrente{
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
            String categoria;
            while(true){
                String buff;
                System.out.println("Inserire la categoria; \"fine\" per uscire");
                categoria = tastiera.readLine();
                if(!categoria.equals("fine")){
                    outSock.write(categoria);
                    //outSock.newLine(); non ci va per il discorso del /n a
                    outSock.flush();
                    while (!(buff = inSock.readLine()).equals("--END CONNECTION--")) {
                        System.out.println(buff);
                    }
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
