package it.unife.reti.grpc.example;

import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.Status;
import io.grpc.StatusRuntimeException;
import it.unife.reti.grpc.example.RemoteLsGrpc;
import it.unife.reti.grpc.example.RemoteLsGrpc.RemoteLsBlockingStub;
import it.unife.reti.grpc.example.RemoteLsOuterClass.DirName;
import it.unife.reti.grpc.example.RemoteLsOuterClass.FileList;
import it.unife.reti.grpc.example.RemoteLsOuterClass;

public class RemoteLsClient {

    private final ManagedChannel channel;
    private final RemoteLsBlockingStub blockingStub;

    // Definiamo il costruttore che prende in ingresso host e porta
    public RemoteLsClient(String host, int port) {
        // usePlaintext e' importante, qui specifichiamo di non usare tls
        // nella conessione tra stub e skeleton
        this(ManagedChannelBuilder.forAddress(host, port).usePlaintext());
    }

    public RemoteLsClient(ManagedChannelBuilder<?> channelBuilder) {
        channel = channelBuilder.build();
        // in questo caso lo stub e' bloccante e non asincrono
        blockingStub = RemoteLsGrpc.newBlockingStub(channel);
    }

    // metodo gRPC implementazione client
    public void ls(String directory) {
        DirName dir = DirName.newBuilder().setName(directory).build();
        FileList filelist = blockingStub.ls(dir);
        for (int i = 0; i < filelist.getFileCount(); i++) {
            System.out.println("" + filelist.getFile(i));
        }
    }

    public static void main (String[] args) {
        if (args.length != 2) {
            System.err.print("Error, usage: RemoteLsClient <hostname> <dirname>");
            System.exit(-1);
        }
        String dirname = args[1];
        RemoteLsClient client = new RemoteLsClient(args[0], 8081);
        client.ls(dirname);
    }
}
