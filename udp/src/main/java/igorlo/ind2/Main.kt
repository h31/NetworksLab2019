package igorlo.ind2

object Main {

    @JvmStatic
    fun main(args: Array<String>) {
        Thread(Runnable {
            Server().run()
        }).start()
        Thread.sleep(500)
        Thread(Runnable {
            Client().run()
        }).start()
    }

}