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
        try (final var x = Shared.open(X.class, "hello", 1, 2, null, new int[3])) {
            System.out.println(x.a);
            System.out.println(x.b);
            System.out.println(x.obj);
            System.out.println(x.arr);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
