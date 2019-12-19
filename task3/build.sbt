lazy val baseSettings = Seq(
    name := "tftp"
  , organization := "ru.polytech.kspt"
  , version := "0.1.0-SNAPSHOT"
  , scalaVersion := "2.12.8"
  , sbtVersion := "1.2.8"
  , scalacOptions ++= Seq(
      "-feature"
    , "-unchecked"
    , "-deprecation"
    , "-Xfatal-warnings"
    , "-Ypartial-unification"
    , "-language:higherKinds"
  )
  , resourceDirectory in Compile := baseDirectory.value / "resources"
)

lazy val pureConfigVersion = "0.12.0"
lazy val scalaTestVersion  = "3.0.4"
lazy val logbackVersion    = "1.2.3"
lazy val catsVersion       = "2.0.0"
lazy val fs2Version        = "2.1.0"

lazy val deps = Seq(
    "org.typelevel"         %% "cats-effect"    % catsVersion
  , "org.typelevel"         %% "cats-core"      % catsVersion
  , "org.scalatest"         %% "scalatest"      % scalaTestVersion
  , "ch.qos.logback"        % "logback-classic" % logbackVersion
  , "com.github.pureconfig" %% "pureconfig"     % pureConfigVersion
  , "co.fs2"                %% "fs2-core"       % fs2Version
  , "co.fs2"                %% "fs2-io"         % fs2Version
).map(_ withSources () withJavadoc ())

lazy val assemblySettings = Seq(
    assemblyJarName in assembly := name.value + ".jar"
  , assemblyMergeStrategy in assembly := {
    case PathList("META-INF", xs @ _*) => MergeStrategy.discard
    case _                             => MergeStrategy.first
  }
)

lazy val global = (project in file("."))
  .settings(baseSettings)

lazy val server = project
  .settings(baseSettings, assemblySettings, libraryDependencies ++= deps)
  .dependsOn(utils)

lazy val client = project
  .settings(baseSettings, assemblySettings, libraryDependencies ++= deps)
  .dependsOn(utils)

lazy val utils = project
  .settings(baseSettings, libraryDependencies ++= deps)
