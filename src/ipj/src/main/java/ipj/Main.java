package ipj;

class X extends Shared {
    private static final long IPJ_VERSION = 34511L;

    public int a, b;
    public Shared obj;
    public int[] arr;

    public X(Integer a, Integer b, Shared obj, int[] arr) {
        this.a = a;
        this.b = b;
        this.obj = obj;
        this.arr = arr;
    }
}

public class Main {
    public static void main(String[] args) {
        System.out.println("args.length=" + args.length);
        for (var i = 0; i < args.length; ++i)
            System.out.println("args[" + i + "]=" + args[i]);

        try (
            final var x1 = Shared.open(X.class, "hello", 1, 2, null, new int[3]);
            final var x2 = Shared.open(X.class, "hello", 3, 4, null, new int[2])
        ) {
            System.out.println(x1.a);
            System.out.println(x1.b);
            System.out.println(x1.obj);
            System.out.println(x1.arr);

            if (args.length >= 2 && "set".startsWith(args[0])) {
                x1.set(0, Integer.valueOf(args[1]));
                System.in.read();
            }
            System.out.println(x2.getInt(0));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
