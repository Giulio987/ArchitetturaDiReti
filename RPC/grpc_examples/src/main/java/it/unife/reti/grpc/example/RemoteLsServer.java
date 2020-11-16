package it.unife.reti.grpc.example;

/* 
 * Lista di import necessari
 * per implementare il server del nostro 
 * servizio di RemoteLs
 */
import io.grpc.Server;
import io.grpc.ServerBuilder;
import io.grpc.stub.StreamObserver;
import java.io.IOException;
import java.io.File;
import it.unife.reti.grpc.example.RemoteLsOuterClass.DirName;
import it.unife.reti.grpc.example.RemoteLsOuterClass.FileList;

public class RemoteLsServer {
	
	// per sempilicita' non verranno
	// definiti i metodi per re-implementare start stop e
	// shutdown del servizio, che da buona prassi dovrebbero
	// essere implementati!

	// Server e porta GRPC su cui girera' il servizio
	private Server server = null;
	private final int port;

	
	// questa e' la classe core che implementa i metodi definiti 
	// dal servizio
	static class RemoteLsImplementation extends RemoteLsGrpc.RemoteLsImplBase {

		@Override
		public void ls(DirName dirname, StreamObserver<FileList> responseObserver) {
			String name = dirname.getName();
			System.out.println("Serving request for directory: " + name);
			//eventuali null check
			File dir = new File(name);
			String files[] = dir.list();
			FileList.Builder flb = FileList.newBuilder();
			for (String file: files) {
				//System.out.println(file);
				flb.addFile(file);
			}
			responseObserver.onNext(flb.build());
			responseObserver.onCompleted();
		}
	}

	// definiamo un costruttore per il nostro server
	public RemoteLsServer(int port) {
		this.port = port;
		ServerBuilder sb = ServerBuilder.forPort(port)
					.addService(new RemoteLsImplementation());
		server = sb.build();
	}

	private void start() throws IOException {
		if (server != null) {
			server.start();
		}
	}

	private void stop() throws IOException {
		if (server != null) {
			server.shutdown();
		}
	}

	// metodo che consente al server GRPC di sincronizzarsi con il main thread
	public void blockUntilShutDown() throws InterruptedException {
		if (server != null) {
			server.awaitTermination();
		}
	}

	// definiamo il main della nostra classe pubblica
	public static void main(String[] args) throws IOException, InterruptedException {
		final RemoteLsServer rlsServer = new RemoteLsServer(8081);
		rlsServer.start();
		rlsServer.blockUntilShutDown();
	}
}
