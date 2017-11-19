import org.jctools.queues.SpscArrayQueue;

import java.io.*;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class TestJavaClient {
    public static void main(String[] args) throws Exception {

        //final SpscArrayQueue<Long> queue = new SpscArrayQueue<Long> (1024);
        final ArrayBlockingQueue<Long> queue = new ArrayBlockingQueue<Long>(1024);

        Socket socket = null;

        try {
            socket = new Socket( args[0], Integer.parseInt(args[1]));
            final DataOutputStream os = new DataOutputStream(socket.getOutputStream());
            final DataInputStream is = new DataInputStream(socket.getInputStream());

            new Thread(new Runnable(){

                public void run() {
                    while (true) {
                        Long v = queue.poll();
                        if ( v != null) {
                            try {
                                os.writeLong(v);

                                long getValue = is.readLong();

                                System.out.println("elpased:" + (System.nanoTime() - getValue));
                            } catch (IOException e) {
                                e.printStackTrace();
                            }
                        }

                    }
                }
            }).start();
            for (int i = 0; i < 100000; ++i) {
                long time = System.nanoTime();
                while (!queue.offer(time)) {
                    Thread.yield();
                }
                Thread.sleep(1000);
            }
        } finally {

            if (socket != null) socket.close();
        }
    }
}
