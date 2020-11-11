import java.io.*;
import java.net.*;

class RemoteSquareServer{
    public static void main(String args[]){
        if(args.length != 1){
            System.err.println("Inserire il numero di porta corretto");
            System.exit(1);
        }
        try{
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));
            while(true){
                Socket s = ss.accept();
                BufferedReader readS = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
                BufferedWriter writeS = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(),"UTF-8"));
                int number;
                String line, result;
                while ((line = readS.readLine()) != null){
                    try{
                        number = Integer.parseInt(line);
                    }catch(NumberFormatException es){
                        System.err.println("Numero ricevuto non valido " +line);
                        continue;
                    }
                    result = ""+(number*number);
                    System.out.println("Risultato:" + result);
                    writeS.write(result);
                    writeS.newLine();
                    writeS.flush();
                }
                s.close();
            }
        }catch(IOException e){
            e.getMessage();
            System.exit(1);
        }
        
    }
}