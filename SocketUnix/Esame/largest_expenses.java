import java.io.*;
import java.net.*;

public class largest_expenses
{
    public static void main(String args[])
    {
        if (args.length != 2) {
            System.err.println("Errore! La sintassi corretta è: java largest_expenses hostname porta");
            System.exit(1);
        }

        try {
            Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]));

            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(),"UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(theSocket.getOutputStream(),"UTF-8"));

            for (;;) {
                System.out.println("Inserisci l'anno di interesse, 'fine' per uscire: ");
                String anno = userIn.readLine();
                if (anno.equals("fine")){
                    System.out.println("Hai scelto di terminare il programma.");
                    break;
                }
                try{
                    Integer.parseInt(anno);
                }catch(NumberFormatException e){
                    System.err.println("Inserire un anno valido");
                    break;
                }
                networkOut.write(anno);
                networkOut.newLine();
                networkOut.flush();
                //CONTROLLO ACK
                String buff = networkIn.readLine();
                if(!buff.equals("ack")){
                    System.err.println("Errore lettura Ack dal server");
                    System.exit(2);
                }
                //Numero di spese
                System.out.println("Inserisci il numero di spese di interesse, 'fine' per uscire: ");
                String N = userIn.readLine();
                if (N.equals("fine")){
                    System.out.println("Hai scelto di terminare il programma.");
                    break;
                }
                try{
                    Integer.parseInt(N);
                }catch(NumberFormatException e){
                    System.err.println("Inserire un numero valido");
                    break;
                }
                networkOut.write(N);
                networkOut.newLine();
                networkOut.flush();

                //CONTROLLO ACK
                buff = networkIn.readLine();
                if(!buff.equals("ack")){
                    System.err.println("Errore lettura Ack dal server");
                    System.exit(2);
                }
                //Tipolgia di spese
                System.out.println("Inserisci la tipologia di spese, 'fine' per uscire: ");
                String tipologia = userIn.readLine();
                
                if (tipologia.equals("fine")){
                    System.out.println("Hai scelto di terminare il programma.");
                    break;
                }
                networkOut.write(tipologia);
                networkOut.newLine();
                networkOut.flush();

                for (;;) {
                    //Leggo input da Server riga per riga 
                    buff = networkIn.readLine();

                    /* Controllo errori */
                    if (buff == null) {
                        System.err.println("Errore! Il Server ha chiuso la connessione!");
                        System.exit(2);
                    }

                    /* Stampo riga letta da Server */
                    System.out.println(buff);

                    /* Passo a nuova richiesta una volta terminato input Server */
                    if (buff.equals("--END--")) {
                        break;
                    }
                }
            }

            /* chiudo la socket! */
            theSocket.close();
        }
        catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
