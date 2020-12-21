import java.io.*;
import java.net.*;

public class ClientMagazzino
{
    public static void main(String args[])
    {
        if (args.length != 2) {
            System.err.println("Errore! La sintassi corretta è: java ClientMagazzino hostname porta");
            System.exit(1);
        }

        try {
            Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]));

            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(),"UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(new OutputStreamWriter(theSocket.getOutputStream(),"UTF-8"));

            for (;;) {
                System.out.println("Inserisci il nome di un vino cui sei interessato 'fine' per uscire: ");
                String nomeVino = userIn.readLine();

                if (nomeVino.equals("fine")){
                    System.out.println("Hai scelto di terminare il programma.");
                    break;
                }
                networkOut.write(nomeVino);
                //SERVE PER rxb
                networkOut.newLine();
                networkOut.flush();

                //CONTROLLO ACK
                String buff = networkIn.readLine();
                if(!buff.equals("ack")){
                    System.err.println("Errore lettura Ack dal server");
                    System.exit(2);
                }

                System.out.println("Inserisci l'annata di un vino cui sei interessato 'fine' per uscire): ");
                String annata = userIn.readLine();
                
                if (annata.equals("fine")){
                    System.out.println("Hai scelto di terminare il programma.");
                    break;
                }
                networkOut.write(annata);
                networkOut.newLine();
                networkOut.flush();

                /* Leggo la risposta del server e la stampo a video */
                for (;;) {
                    /* Leggo input da Server riga per riga */
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

            /* Ricordarsi sempre di chiudere la socket! */
            theSocket.close();
        }
        catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
